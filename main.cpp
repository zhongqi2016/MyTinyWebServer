
#include "WebServer.h"

int main(int argc,char* argv[]) {
    WebServer myServ=WebServer(63343);
    myServ.init();
    myServ.start();
    return 0;
}
