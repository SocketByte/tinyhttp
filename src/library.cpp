#include "library.h"

#include <utility>

std::vector<std::string> split(std::string_view i_str, std::string_view i_delim) {
    std::vector<std::string> result;

    size_t found = i_str.find(i_delim);
    size_t startIndex = 0;

    while (found != std::string::npos) {
        std::string temp(i_str.begin()+startIndex, i_str.begin()+found);
        result.push_back(temp);
        startIndex = found + i_delim.size();
        found = i_str.find(i_delim, startIndex);
    }
    if (startIndex != i_str.size())
        result.emplace_back(i_str.begin()+startIndex, i_str.end());
    return result;
}

std::string respond(int code, std::string_view rsn, std::string_view content_type, std::string_view body) {
    std::stringstream res;
    res << "HTTP/1.1 " << code << " " << rsn << "\n";
    res << "Content-Type: " << content_type << "\n";
    res << "Content-Length: " << body.length() << "\n\n";
    res << body;
    return std::string(res.str());
}

void TinyHttp::run(int port) {
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(1, 1), &wsa_data);
#endif
    if (debug)
        std::cout << "success: running web server on port " << port << std::endl;


    SOCKET sockfd;
    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(port);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0) {
        std::cout << "error: socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (debug)
        std::cout << "success: binding socket server" << std::endl;


    if (bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "error: socket binding failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    listen(sockfd, SOMAXCONN);
    if (debug)
        std::cout << "success: running listener" << std::endl;


    fd_set readfds;
    std::vector<SOCKET> clientfds;

    // recv buffer
    int recvbufsize = 1024;
    char recvbuf[recvbufsize];

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        for (SOCKET clientfd : clientfds) {
            if (clientfd > 0) {
                FD_SET(clientfd, &readfds);
            }
        }

        select(0, &readfds, nullptr, nullptr, nullptr);

        if (FD_ISSET(sockfd, &readfds)) {
            SOCKET clientfd = accept(sockfd, nullptr, nullptr);

#ifdef _WIN32
            if (clientfd != INVALID_SOCKET) {
#else
            if (clientfd >= 0) {
#endif
                clientfds.push_back(clientfd);
                if (debug)
                    std::cout << "success: handled client connection" << std::endl;
            }
        }

        for (int i = 0; i < clientfds.size(); i++) {
            SOCKET clientfd = clientfds[i];
            if (FD_ISSET(clientfd, &readfds)) {
                int rec = recv(clientfd, recvbuf, recvbufsize, 0);
                if (rec == 0 || rec == SOCKET_ERROR) {
                    clientfds.erase(clientfds.begin() + i);
#ifdef _WIN32
                    closesocket(clientfd);
#else
                    close(clientfd);
#endif
                    if (debug)
                        std::cout << "success: handled client disconnection" << std::endl;
                    break;
                }

                handle(clientfd, std::string(recvbuf));
            }
        }
    }
}

void TinyHttp::make_route(const char* path, std::function<void(Context)> function) {
    Route route;
    route.job = std::move(function);
    route.path = std::string(path);

    this->routes.push_back(route);
}

void TinyHttp::handle(int clientfd, std::string_view recv) {
    Context ctx;
    Response resp;
    ctx.response = &resp;

    std::string base = split(recv, "\n")[0];
    std::vector<std::string> data = split(base, " ");

    std::string method = data[0];
    std::string path = data[1];

    std::vector<std::string> query = split(path, "?");
    path = query[0];

    ctx.method = method;
    ctx.path = path;

    if (debug)
        std::cout << "success: clientfd " << clientfd << " -> " << ctx.method << " " << ctx.path << std::endl;

    if (query.size() > 1) {
        std::vector<std::string> pms = split(query[1], "&");

        std::map<std::string, std::string> params;
        for (const auto& prm : pms) {
            std::vector<std::string> s = split(prm, "=");
            params[s[0]] = s[1];
        }
        ctx.params = params;
    }

    for (const auto& route : this->routes) {
        if (route.path != ctx.path) {
            continue;
        }
        route.job(ctx);

        std::string res = respond(200, "OK", ctx.response->content_type, ctx.response->body);

        send(clientfd, res.c_str(), res.length(), 0);
    }
}

void Response::write_text(const char *text) {
    this->content_type = "text/plain";
    this->body = std::string(text);
}

void Response::write_html(const char *html) {
    this->content_type = "text/html";
    this->body = std::string(html);
}
