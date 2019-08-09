#ifndef TINYHTTP_LIBRARY_HPP
#define TINYHTTP_LIBRARY_HPP

#include <iostream>
#include <functional>
#include <map>
#include <iterator>
#include <vector>
#include <sstream>

#ifdef _WIN32
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
  #endif
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>

  typedef int SOCKET;
#endif

#ifdef SOCKET_ERROR
//
#else
#define SOCKET_ERROR -1
#endif

class Response {
public:
    std::string body;
    std::string content_type;

    void write_text(const char *text);
    void write_html(const char *html);
};

class Context {
public:
    std::string method;
    std::string path;
    std::map<std::string, std::string> params;

    Response *response;
};

class Route {
public:
    std::string path;
    std::function<void(Context)> job;
};

class TinyHttp {
public:
    bool debug = true;

    void run(int port);
    void make_route(const char *path, std::function<void(Context)>);
private:
    std::vector<Route> routes;

    void handle(int clientfd, std::string_view recv);
};

#endif