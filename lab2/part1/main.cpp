#include <chrono>
#include <thread>
#include <stdio.h>
#include <string.h>
#include <iostream>
using namespace std::chrono_literals;

#include <cpprest/http_msg.h>
#include <wiringPi.h>

#include "metronome.hpp"
#include "rest.hpp"

// ** Remember to update these numbers to your personal setup. **
#define LED_RED   20
#define LED_GREEN 21
#define BTN_MODE  19
#define BTN_TAP   26

metronome my_metronome;  //initialize: m_beat_count = 0, m_timing = false

size_t prev_bpm = 0, max_bpm = 0, min_bpm = INT_MAX;

size_t current_bpm = 0;

//start learn mode
void metronome::start_timing()
{
	m_beat_count = 0;
	m_timing = true;//start learn mode

    my_metronome.set_timing(m_timing);
}

//stop learn mode, go to "play" mode
void metronome::stop_timing()
{
	m_timing = false;//stop  learn  mode
	my_metronome.set_timing(m_timing);
	size_t curr_bpm = 0;
	curr_bpm = my_metronome.get_bpm();
	
	
	printf("CURRENT BPM:- %d\n", curr_bpm);
	//bpm_changed = 1;
	if (curr_bpm !=0 && curr_bpm < min_bpm) {
		min_bpm = curr_bpm;
	}
	if ( curr_bpm !=0 && curr_bpm > max_bpm) {
		max_bpm = curr_bpm;
	}
	if (curr_bpm != 0) {
		current_bpm = curr_bpm;
		//get the tempo into blink (in play mode)
	}
}
void metronome::tap()
{
	size_t time_current = millis();
	if (m_beat_count >= 4)
	{
		for (int i = 0; i < m_beat_count-1; i++)
		{
			m_beats[i] = m_beats[i+1];
		}
		m_beats[3] = time_current;

	}
	else
		m_beats[m_beat_count++] = time_current;
    
	printf("time  %d\n", time_current);
}

size_t metronome::get_bpm() const {
	int i = 0;
	size_t avg_bpm = 0;
	size_t sum_bpm = 0;
	if (m_beat_count < 4) {
		return prev_bpm;
	}
	else {
		for (i = 0; i < m_beat_count-1 ; i++) {
			sum_bpm = sum_bpm + m_beats[i+1]- m_beats[i ];
		}
		
		avg_bpm = (size_t)sum_bpm / (m_beat_count-1 );

		prev_bpm = avg_bpm;
		return 60000/prev_bpm;
	}
}

//toggle the mode
void on_mode()
{
	//Go to play mode
	if (my_metronome.is_timing() == true)
	{
		digitalWrite(LED_RED, LOW);
		
		my_metronome.stop_timing();
	}
	else
	{
		//Go to learn mode
		digitalWrite(LED_GREEN, LOW);

		my_metronome.start_timing();
	}
}

void on_tap()
{
	// Receive a tempo tap
	if (my_metronome.is_timing()==true)//at "learn" mode
	{
		my_metronome.tap();
		
	}
}
// Mark as volatile to ensure it works multi-threaded.
volatile bool btn_mode_pressed = false;
volatile bool btn_tap_pressed = false;

volatile bool tap_button_state = false;
volatile bool tap_last_state = false;

//Run on tap button (learn or play mode run)
void run_on_tap()
{
	while (true)
	{
		//in learn mode
		if (my_metronome.is_timing() == true)
		{
			tap_button_state = digitalRead(BTN_TAP);
			if (tap_button_state != tap_last_state)
			{
				delay(20);
				if (tap_button_state == HIGH)
				{
					digitalWrite(LED_RED, HIGH );
					on_tap();
				}
				else
				{
					digitalWrite(LED_RED, LOW );
				}
			}
			tap_last_state = tap_button_state;
		}
		//in play mode
		else {
			size_t wait_time ;
			if(current_bpm !=0)
			{
				wait_time = 60000/current_bpm;
			    digitalWrite(LED_GREEN, LOW);
			    if(wait_time > 300){
					std::this_thread::sleep_for(std::chrono::milliseconds(wait_time-200));
				
					//change mode when LED is playing
					if(my_metronome.is_timing() == true)
					{
						continue;
					}				
				
					digitalWrite(LED_GREEN, HIGH);   //led on
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}
			    else{	
				    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time/2));
				    if(my_metronome.is_timing() == true)
					{
						continue;
					}	
					digitalWrite(LED_GREEN, HIGH);   //led on
					std::this_thread::sleep_for(std::chrono::milliseconds(wait_time/2));
				}
			}
		    else
			{
			    digitalWrite(LED_GREEN, LOW);
		    }
		}
	}

}

volatile bool mode_last_state = false;
volatile bool mode_button_state = false;

// This is called when a GET request is made to "/answer".
void get_restbpm(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.set_body(web::json::value::number(current_bpm));
	msg.reply(response);
}

void put_restbpm(web::http::http_request msg) {
	size_t input_bpm;
	std::string input_str = msg.content_ready().get().extract_json(true).get().serialize();
	input_bpm = std::stoi(input_str);
	//change current bpm
	current_bpm = input_bpm;
	//may change min or max bpm
	if (current_bpm !=0 && current_bpm < min_bpm) {
		min_bpm = current_bpm;
	}
	if ( current_bpm !=0 && current_bpm > max_bpm) {
		max_bpm = current_bpm;
	}
	
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.set_body(web::json::value::string("test"));
	msg.reply(response);
}

void get_min(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.set_body(web::json::value::number(min_bpm));
	msg.reply(response);
}

void delete_min(web::http::http_request msg) {
	std::cout << min_bpm << std::endl;

	min_bpm = INT_MAX;
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.set_body(web::json::value::string("delete successfully!"));
	msg.reply(response);
}
void get_max(web::http::http_request msg) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.set_body(web::json::value::number( max_bpm));
	msg.reply(response);
}

void delete_max(web::http::http_request msg) {
	std::cout <<  max_bpm << std::endl;
	max_bpm = 0;
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	response.set_body(web::json::value::string("delete successfully!"));
	msg.reply(response);
}
// This program shows an example of input/output with GPIO, along with
// a dummy REST service.
// ** You will have to replace this with your metronome logic, but the
//    structure of this program is very relevant to what you need. **
int main() {
	// WiringPi must be initialized at the very start.
	// This setup method uses the Broadcom pin numbers. These are the
	// larger numbers like 17, 24, etc, not the 0-16 virtual ones.
	wiringPiSetupGpio();

	// Set up the directions of the pins.
	// Be careful here, an input pin set as an output could burn out.
	pinMode(LED_RED, OUTPUT);
	pinMode(BTN_MODE, INPUT);

	pinMode(LED_GREEN, OUTPUT);
	pinMode(BTN_TAP, INPUT);
	
	// Note that you can also set a pull-down here for the button,
	// if you do not want to use the physical resistor.
	//pullUpDnControl(BTN_MODE, PUD_DOWN);

	// Configure the rest services after setting up the pins,
	// but before we start using them.
	// ** You should replace these with the BPM endpoints. **
	auto bpm_rest = rest::make_endpoint("/bpm");
	bpm_rest.support(web::http::methods::GET, get_restbpm);
	bpm_rest.support(web::http::methods::PUT, put_restbpm);
	
	auto bpm_min_rest = rest::make_endpoint("/bpm/min");
	bpm_min_rest.support(web::http::methods::GET, get_min);
	bpm_min_rest.support(web::http::methods::DEL, delete_min);

	auto bpm_max_rest = rest::make_endpoint("/bpm/max");
	bpm_max_rest.support(web::http::methods::GET, get_max);
	bpm_max_rest.support(web::http::methods::DEL, delete_max);

	// Start the endpoints in sequence.
	bpm_rest.open().wait();
	bpm_min_rest.open().wait();
	bpm_max_rest.open().wait();
	
	// Use a separate thread for the blinking.
	// This way we do not have to worry about any delays
	// caused by polling the button state / etc.

	std::thread run_on_tap_thread(run_on_tap);
	run_on_tap_thread.detach();

	// ** This loop manages reading button state. **
	while (true) {

		mode_button_state = digitalRead(BTN_MODE);
		if (mode_button_state != mode_last_state)
		{
	        delay(20);
			if (mode_button_state == HIGH)
			{
				on_mode();
			}
		}
		mode_last_state = mode_button_state;
		// We are using a pull-down, so pressed = HIGH.
		// If you used a pull-up, compare with LOW instead.
	//	btn_mode_pressed = (digitalRead(BTN_MODE) == HIGH);
		// Delay a little bit so we do not poll too heavily.
		
	//	std::this_thread::sleep_for(10ms);
		// ** Note that the above is for testing when the button
		// is held down. For detecting "taps", or momentary
		// pushes, you need to check for a "rising edge" -
		// this is when the button changes from LOW to HIGH... **
	}

	return 0;
}
