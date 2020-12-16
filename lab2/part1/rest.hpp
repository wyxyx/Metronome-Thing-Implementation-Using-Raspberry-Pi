#include <string>

#include <cpprest/http_listener.h>

using http_listener = web::http::experimental::listener::http_listener;

class rest {
public:
        static http_listener make_endpoint(const std::string& path) {
                web::uri_builder builder;
                builder.set_scheme("http");
                builder.set_host("0.0.0.0");
                builder.set_port(8080);
                builder.set_path(path);

                auto listener = http_listener(builder.to_uri());
                listener.support(web::http::methods::OPTIONS, allowAll);

                return listener;
        }

private:
        static void allowAll(web::http::http_request msg) {
                web::http::http_response response(200);
                response.headers().add("Access-Control-Allow-Origin", "*");
                response.headers().add("Access-Control-Allow-Methods", "GET, PUT, DELETE");
                response.headers().add("Access-Control-Allow-Headers", "Content-Type");

                msg.reply(response);
        }

private:
        rest();
};
