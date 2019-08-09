## TinyHttp

Very, very small and basic cross-platform http web server.  
Example:

```cpp
#include <iostream>
#include "library.hpp"

int main() {
    TinyHttp web;
    web.make_route("/", [](Context ctx) {
        ctx.response->write_text("Hello, World!");
    });
    web.run(80);
    return 0;
}
```