#pragma once
#include <time.h>
#include <cstddef>

class metronome
{
public:
	enum { beat_samples = 4 };

public:
	metronome()
	: m_timing(false), m_beat_count(0) {}
	~metronome() {}

public:
	// Call when entering "learn" mode
	void start_timing();
	// Call when leaving "learn" mode
	void stop_timing();

	// Should only record the current time when timing
	// Insert the time at the next free position of m_beats
	void tap();

    void set_timing(bool m_time){m_timing = m_time;}
	volatile bool is_timing() const { return m_timing; }
	// Calculate the BPM from the deltas between m_beats
	// Return 0 if there are not enough samples
	size_t get_bpm() const;

private:
	volatile bool m_timing;

	time_t timer_start;
	time_t timer_stop;

	// Insert new samples at the end of the array, removing the oldest
	size_t m_beats[beat_samples];
	size_t m_beat_count;
};

