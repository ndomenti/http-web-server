#include "net/Server.hpp"

int main() {
    Server server(80);
    server.run();
    return 0;
}