#include "../../include/Global.hpp"
#include "../../include/tcp/Acceptor.hpp"
#include "../../include/event/EventLoop.hpp"
#include "../../include/tcp/TcpSocket.hpp"
#include "../../include/event/Channel.hpp"
#include "../../include/SocketAddress.hpp"

namespace Collie {
namespace Tcp {

Acceptor::Acceptor(std::shared_ptr<SocketAddress> addr) : localAddr(addr) {
    Log(TRACE) << "Acceptor constructing";
}

Acceptor::~Acceptor() { Log(TRACE) << "Acceptor destructing"; }

int
Acceptor::getSocketFd() const {
    return tcpSocket->getFd();
}

void
Acceptor::socket() {
    // create socket and listen
    tcpSocket.reset(new TcpSocket(localAddr));
    tcpSocket->listen();
}

std::shared_ptr<Event::Channel>
Acceptor::getBaseChannel() {
    if(!tcpSocket) socket();

    // create channel
    std::shared_ptr<Event::Channel> channel(
        new Event::Channel(tcpSocket->getFd()));
    channel->setAfterSetLoopCallback([channel, this]() {
        channel->enableRead();
        channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
        channel
            ->enableOneShot(); // NOTE One shot, channel needs to update after
        // every accepting
        channel->setUpdateAfterActivate(true);
    });
    return channel;
}

void
Acceptor::handleRead() {
    // should be thread-safe
    std::shared_ptr<SocketAddress> remoteAddr(new SocketAddress);
    int connFd = tcpSocket->accept(remoteAddr);
    if(connFd > 0) {
        acceptCallback(connFd, remoteAddr);
    } else {
        handleError();
    }
}

void
Acceptor::handleError() {
    // TODO
    Log(TRACE) << "Acceptor handle error";
}
}
}
