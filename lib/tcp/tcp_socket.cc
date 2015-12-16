#include <netinet/tcp.h>
#include <unistd.h>
#include "../../include/tcp/tcp_socket.h"
#include "../../include/inet_address.h"
#include "../../include/utils/file.h"
#include "../../include/logging.h"

namespace collie {
namespace tcp {

TCPSocket::TCPSocket(std::shared_ptr<InetAddress> addr) noexcept
    : Descriptor(-1, false),
      state_(State::Init),
      address_(addr) {
  CreateImpl();
}

TCPSocket::TCPSocket(const int fd, std::shared_ptr<InetAddress> addr) noexcept
    : Descriptor(fd, true),
      state_(State::Accept),
      address_(addr) {}

TCPSocket::TCPSocket() noexcept : Descriptor(-1, false),
                                  state_(State::IllegalAccept) {}

TCPSocket::~TCPSocket() noexcept { CloseImpl(); }

void TCPSocket::Create() noexcept { CreateImpl(); }

void TCPSocket::Close() noexcept { CloseImpl(); }

void TCPSocket::CreateImpl() noexcept {
  if (is_init_) return;
  fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd_ == -1) {
    LOG(WARNING) << "TCPSocket: " << GetSystemError();
  } else {
    state_ = State::Socket;
    is_init_ = true;
  }
}

void TCPSocket::CloseImpl() noexcept {
  if (state_ != State::Init && state_ != State::Close) {
    ::close(fd_);
    state_ = State::Close;
    is_close_ = true;
  }
}

void TCPSocket::SetNoDelay() const {
  CHECK(state_ != State::Init && state_ != State::IllegalAccept &&
        state_ != State::Close);
  int value = 1;
  const int ret =
      ::setsockopt(fd_, SOL_TCP, TCP_NODELAY, &value, sizeof(value));
  if (ret == -1) {
    LOG(WARNING) << "setsockopt() setting TCP_NODELAY: " << GetSystemError();
  }
}

// Throws if `address_` is NULL.
// Listens ip according to ip version, returns true when the operation succeeds.
// If `state_` == `State::Socket`, binds and listens it
// If `state_` == `State::Bind`, listens it
// If `state_` == `State::Listen`, returns true directly
bool TCPSocket::BindAndListen() {
  CHECK(address_);
  switch (address_->ip_version()) {
    case IP::V4:
      return ListenV4();
    default:
      return false;
  }
}

// Listens ipv4, returns true when the operation succeeds
// if `state_` == `State::Socket`, binds and listens it
// if `state_` == `State::Bind`, listens it
// if `state_` == `State::Listen`, returns true directly
bool TCPSocket::ListenV4() {
  if (state_ == State::Socket) {
    struct sockaddr_in servAddr = address_->addr_v4();

    int ret = bind(fd_, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if (ret == -1) {
      LOG(WARNING) << "bind()" << GetSystemError();
    } else {
      state_ = State::Bind;
    }
  }
  if (state_ == State::Bind) {
    if (::listen(fd_, SOMAXCONN) == -1) {
      LOG(WARNING) << GetSystemError();
    } else {
      state_ = State::Listen;
    }
  }
  if (state_ == State::Listen) {
    return true;
  } else {
    return false;
  }
}

bool TCPSocket::Connect(std::shared_ptr<InetAddress> serv_address) {
  CHECK(serv_address);
  switch (serv_address->ip_version()) {
    case IP::V4:
      return ConnectV4(serv_address);
    case IP::V6:
      LOG(WARNING) << "IP v6 is not ready to go";
      return false;
    default:
      LOG(ERROR) << "IP version is UNKNOWN " << serv_address->ip();
      return false;
  }
}

bool TCPSocket::ConnectV4(std::shared_ptr<InetAddress> serv_address) {
  struct sockaddr_in serv = serv_address->addr_v4();
  if (state_ == State::Socket) {
    int ret = ::connect(fd_, (struct sockaddr *)&serv, sizeof(serv));
    if (ret == -1) {
      LOG(WARNING) << "connect()" << GetSystemError();
    } else {
      state_ = State::Connect;

      // get local address
      struct sockaddr_in local;
      int addrLen = sizeof(local);
      ::getsockname(fd_, (struct sockaddr *)&local, (socklen_t *)&addrLen);
      address_ = std::make_shared<InetAddress>(local);
      return true;
    }
  } else {
    LOG(WARNING) << "TCP socket state is not able to connect" << int(state_);
  }
  return false;
}

// Thread safe
std::shared_ptr<TCPSocket> TCPSocket::Accept(bool blocking) {
  CHECK(address_);
  switch (address_->ip_version()) {
    case IP::V4:
      return AcceptV4(blocking);
    default:
      return GetIllegalAcceptSocket();
  }
}

std::shared_ptr<TCPSocket> TCPSocket::AcceptV4(bool blocking) {
  if (state_ == State::Listen) {
    int flags = 0;
    if (blocking) flags = SOCK_NONBLOCK;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen;
    clientAddrLen = sizeof(clientAddr);

    const int ret =
        ::accept4(fd_, (struct sockaddr *)&clientAddr, &clientAddrLen, flags);
    if (ret == -1) {
      LOG(TRACE) << "accept(): " << GetSystemError();
      return GetIllegalAcceptSocket();
    } else {
      LOG(DEBUG) << "Socket accept " << ret;
      auto addr = std::make_shared<InetAddress>(clientAddr);
      return GetAcceptSocket(ret, addr);
    }
  }
  return GetIllegalAcceptSocket();
}

std::shared_ptr<TCPSocket> TCPSocket::GetAcceptSocket(
    const int fd, std::shared_ptr<InetAddress> addr) {
  CHECK(fd > 0);
  return std::shared_ptr<TCPSocket>(new TCPSocket(fd, addr));
}

std::shared_ptr<TCPSocket> TCPSocket::GetIllegalAcceptSocket() noexcept {
  static const auto illegalSocket = std::shared_ptr<TCPSocket>(new TCPSocket());
  return illegalSocket;
}
}
}
