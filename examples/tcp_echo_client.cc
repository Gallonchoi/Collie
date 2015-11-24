#include <iostream>
#include <string>
#include "../include/tcp/tcp_client.h"
#include "../include/tcp/tcp_socket.h"
#include "../include/exception.h"
#include "../include/logging.h"

using collie::tcp::TCPClient;
using collie::tcp::TCPSocket;
using namespace collie;

int main(int argc, char* argv[]) {
  auto& logging = logger::LogHandler::GetHandler();
  logging.set_log_level(TRACE);
  logging.Init();
  unsigned port = 8080;
  if (argc == 2) port = std::stoul(argv[1]);

  TCPClient client;
  client.set_connect_callback([](std::shared_ptr<TCPSocket> socket) {
    socket->Send("Hello, here is client");
    std::string content;
    socket->Recv(content);
    Log(INFO) << "Received: " << content;
    socket->Send("Goodbye");
    socket->Recv(content);
    Log(INFO) << "Received: " << content;

  });
  client.Connect("127.0.0.1", port, 4, 100);
  return 0;
}