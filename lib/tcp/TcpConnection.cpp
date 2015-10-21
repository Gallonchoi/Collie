#include "../../include/Global.hpp"
#include "../../include/tcp/TcpConnection.hpp"
#include "../../include/tcp/TcpSocket.hpp"
#include "../../include/SocketAddress.hpp"
#include "../../include/event/EventLoop.hpp"
#include "../../include/event/Channel.hpp"
#include <functional>
#include <algorithm>


namespace Collie { namespace Tcp {

TcpConnection::TcpConnection(std::shared_ptr<Event::EventLoop> eventLoop,
                             const unsigned fd,
                             std::shared_ptr<SocketAddress> localAddr,
                             std::shared_ptr<SocketAddress> remoteAddr) :
    connected(true),
    connFd(fd),
    eventLoop(eventLoop),
    channel(new Event::Channel(eventLoop, connFd)),
    localAddr(localAddr),
    remoteAddr(remoteAddr) {

    channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    channel->enableRead();
    eventLoop->updateChannel(channel);
    Log(TRACE) << "Tcp Connection constructing";
}

TcpConnection::~TcpConnection() {
    Log(TRACE) << "Tcp Connection destructing";
}

void
TcpConnection::disconnect() {
    connected = false;
    channel->remove();
}

std::string
TcpConnection::recvAll() {
    // FIXME
    std::string buffer;
    buffer.swap(inputBuffer);
    return buffer;
}

void
TcpConnection::send(const std::string & buffer) {
    // FIXME
    outputBuffer += buffer;
}

void
TcpConnection::handleRead() {
    const std::string content = TcpSocket::recv(channel->getFd(), 0);
    if(content.size() > 0) {
        inputBuffer += content;
        messageCallback(shared_from_this());
        inputBuffer.clear();
    } else {
        channel->disableRead();
        channel->update();
    }
    if(outputBuffer.size() > 0) {
        channel->enableWrite();
        channel->update();
    }
}

void
TcpConnection::handleWrite() {
    if(outputBuffer.size() > 0) {
        TcpSocket::send(channel->getFd(), outputBuffer, 0);
        outputBuffer.clear();
    }
    channel->disableWrite();
    channel->update();
}

void
TcpConnection::handleClose() {
    disconnect();
}

void
TcpConnection::handleError() {
    disconnect();
}

}}