#include <iostream>
#include "../src/library.hpp"

// Example
int main() {
    TinyHttp web;
    web.debug = false;
    web.make_route("/", [](Context ctx) {
        ctx.response->write_text("Hello, World!");
    });
    web.run(80);
    return 0;
}