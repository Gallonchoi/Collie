#ifndef COLLIE_TCP_CONNECTOR_H
#define COLLIE_TCP_CONNECTOR_H

#include <functional>
#include <memory>

namespace Collie {

namespace Event {

class ThreadPool;

}

class SocketAddress;

namespace Tcp {

class TcpSocket;

class Connector {
public:
    using ConnectCallback = std::function<void(std::shared_ptr<TcpSocket>)>;

    explicit Connector(std::shared_ptr<SocketAddress>);
    Connector(const Connector &) = delete;
    Connector & operator=(const Connector &) = delete;
    ~Connector();

    void setConnectCallback(const ConnectCallback & cb) {
        connectCallback = cb;
    }
    void setConnectCallback(const ConnectCallback && cb) {
        connectCallback = std::move(cb);
    }
    void connect(const size_t threadNum, const size_t connectNum);

private:
    std::unique_ptr<Event::ThreadPool> threadPool;
    std::shared_ptr<SocketAddress> remoteAddr;
    ConnectCallback connectCallback;
};
}
}

#endif /* COLLIE_TCP_CONNECTOR_H */
