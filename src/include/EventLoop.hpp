#pragma once

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::placeholders;

static constexpr auto BUF_SIZE{8192};

namespace internal {

class Session {
public:
  Session(boost::asio::io_service &io_service,
          std::function<void(std::vector<char>)> cb_received,
          std::function<void()> cb_closed)
      : socket_(io_service), cb_received_(cb_received), cb_closed_(cb_closed)
  {
  }

  boost::asio::ip::tcp::socket &socket() { return socket_; }

  void async_read()
  {
    socket_.async_read_some(
        boost::asio::buffer(data_, BUF_SIZE),
        boost::bind(&Session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  }

  void handle_read(const boost::system::error_code &error,
                   std::size_t bytes_transferred)
  {
    if (!error) {
      cb_received_({data_.begin(), data_.begin() + bytes_transferred});
      async_read();
    } else {
      cb_closed_();
      delete this;
    }
  }

private:
  boost::asio::ip::tcp::socket socket_;
  std::function<void(std::vector<char>)> cb_received_;
  std::function<void()> cb_closed_;

  std::array<char, BUF_SIZE> data_{{0}};
};

} // namespace internal

class EventLoop {
public:
  EventLoop(std::uint16_t port)
      : io_service_(),
        acceptor_(io_service_, boost::asio::ip::tcp::endpoint(
                                   boost::asio::ip::tcp::v4(), port))
  {
  }

  void accept(std::function<void(std::vector<char>)> cb_received,
              std::function<void()> cb_closed)
  {
    auto *new_session =
        new internal::Session(io_service_, cb_received, cb_closed);
    acceptor_.async_accept(
        new_session->socket(),
        boost::bind(&EventLoop::handle_accept, this, new_session,
                    boost::asio::placeholders::error, cb_received, cb_closed));
  }

  void handle_accept(internal::Session *new_session,
                     const boost::system::error_code &error,
                     std::function<void(std::vector<char>)> cb_received,
                     std::function<void()> cb_closed)
  {
    if (!error) {
      new_session->async_read();
      new_session = new internal::Session(io_service_, cb_received, cb_closed);
      acceptor_.async_accept(new_session->socket(),
                             boost::bind(&EventLoop::handle_accept, this,
                                         new_session,
                                         boost::asio::placeholders::error,
                                         cb_received, cb_closed));
    } else {
      delete new_session;
    }
  }

  void await() { io_service_.run(); }

private:
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
