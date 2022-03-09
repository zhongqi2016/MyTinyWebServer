
#include "WebServer.h"

int main() {
    WebServer myServ=WebServer(63343);
    myServ.init();
    myServ.start();
    return 0;
}
