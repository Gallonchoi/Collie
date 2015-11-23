#include "../include/tcp/TCPClient.hpp"
#include "../include/tcp/TCPSocket.hpp"
#include "../include/InetAddress.hpp"
#include "../include/Global.hpp"
#include "../include/utils/File.hpp"

using Collie::TCP::TCPClient;
using Collie::TCP::TCPSocket;
using Collie::Utils::File;
using namespace Collie;

int
main(int argc, char * argv[]) {
    auto & logger = Logger::LogHandler::getHandler();
    logger.setLogLevel(TRACE);
    logger.init();

    unsigned port = 8080;
    if(argc == 2) port = std::stoul(argv[1]);

    TCPClient client;
    client.setConnectCallback([](std::shared_ptr<TCPSocket> socket) {

        REQUIRE(socket);
        socket->send("Hello");
        File file("/home/gallon/collie/recv/test.xls", File::Mode::Write, File::Creat);
        const size_t size = 110080;
        Log(INFO) << "File size = " << size;
        Log(INFO) << "Receiving file...";

        socket->recvFile(file, size);
    });
    client.connect("127.0.0.1", port);

    return 0;
}