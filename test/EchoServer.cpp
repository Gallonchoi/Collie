#include <iostream>
#include <string>
#include "../include/tcp/TcpServer.hpp"
#include "../include/tcp/TcpConnection.hpp"
#include "../include/Global.hpp"

using Collie::Tcp::TcpServer;
using Collie::Tcp::TcpConnection;
using namespace Collie;

int
main(int argc, char * argv[]) {

    auto & logger = Logger::LogHandler::getHandler();
    logger.setLogLevel(TRACE);
    logger.init();

    unsigned port = 8080;
    if(argc == 2) port = std::stoul(argv[1]);

    TcpServer server("0.0.0.0", port);
    server.setOnMessageCallback([](std::shared_ptr<TcpConnection> conn) {
        const std::string content = conn->recvAll();

        std::cout << content << std::endl;
        conn->send("Hi, here is server");
    });
    server.start();
    return 0;
}
