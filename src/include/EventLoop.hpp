#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::placeholders;

namespace internal {

class Session {
public:
  Session(boost::asio::io_service &io_service) : socket_(io_service) {}

  boost::asio::ip::tcp::socket &socket() { return socket_; }

  void start() {
    std::cerr << "new session" << std::endl;
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code &error,
                   size_t bytes_transferred) {
    if (!error) {
      std::cerr << "received " << bytes_transferred << " bytes" << std::endl;
    } else {
      delete this;
    }
  }

private:
  boost::asio::ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

} // namespace internal

class EventLoop {
public:
  EventLoop(std::uint16_t port)
      : io_service_(),
        acceptor_(io_service_, boost::asio::ip::tcp::endpoint(
                                   boost::asio::ip::tcp::v4(), port)) {

    auto *new_session = new internal::Session(io_service_);
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&EventLoop::handle_accept, this,
                                       new_session,
                                       boost::asio::placeholders::error));
  }

  void handle_accept(internal::Session *new_session,
                     const boost::system::error_code &error) {
    if (!error) {
      new_session->start();
      new_session = new internal::Session(io_service_);
      acceptor_.async_accept(new_session->socket(),
                             boost::bind(&EventLoop::handle_accept, this,
                                         new_session,
                                         boost::asio::placeholders::error));
    } else {
      delete new_session;
    }
  }

  void await() { io_service_.run(); }

private:
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
