#pragma once

#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/system/error_code.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

static constexpr auto BUF_SIZE{8192};

namespace event_loop {

namespace internal {

class TCPSession {
public:
  TCPSession(boost::asio::io_service &io_service,
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
        boost::bind(&TCPSession::handle_read, this,
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

class TCPReceiver {
public:
  TCPReceiver(boost::asio::io_service &io_service, short port)
      : io_service_(io_service),
        acceptor_(io_service_, boost::asio::ip::tcp::endpoint(
                                   boost::asio::ip::tcp::v4(), port))
  {
  }

  void accept(std::function<void(std::vector<char>)> cb_received,
              std::function<void()> cb_closed)
  {
    auto *new_session =
        new internal::TCPSession(io_service_, cb_received, cb_closed);
    acceptor_.async_accept(
        new_session->socket(),
        boost::bind(&TCPReceiver::handle_accept, this, new_session,
                    boost::asio::placeholders::error, cb_received, cb_closed));
  }

private:
  boost::asio::io_service &io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;

  void handle_accept(internal::TCPSession *new_session,
                     const boost::system::error_code &error,
                     std::function<void(std::vector<char>)> cb_received,
                     std::function<void()> cb_closed)
  {
    if (!error) {
      new_session->async_read();
      new_session =
          new internal::TCPSession(io_service_, cb_received, cb_closed);
      acceptor_.async_accept(new_session->socket(),
                             boost::bind(&TCPReceiver::handle_accept, this,
                                         new_session,
                                         boost::asio::placeholders::error,
                                         cb_received, cb_closed));
    } else {
      delete new_session;
    }
  }
};

class UDPReceiver {
public:
  UDPReceiver(boost::asio::io_service &io_service, short port)
      : io_service_(io_service),
        socket_(io_service, boost::asio::ip::udp::endpoint(
                                boost::asio::ip::udp::v4(), port))
  {
  }

  void listen(std::function<void(std::vector<char>)> cb_received)
  {
    socket_.async_receive_from(
        boost::asio::buffer(data_, BUF_SIZE), sender_endpoint_,
        boost::bind(&UDPReceiver::handle_receive_from, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred, cb_received));
  }

private:
  boost::asio::io_service &io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_endpoint_;

  std::array<char, BUF_SIZE> data_{{0}};

  void handle_receive_from(const boost::system::error_code &error,
                           size_t bytes_recvd,
                           std::function<void(std::vector<char>)> cb_received)
  {
    cb_received({std::begin(data_), std::end(data_)});

    if (!error && bytes_recvd > 0) {
      socket_.async_receive_from(
          boost::asio::buffer(data_, BUF_SIZE), sender_endpoint_,
          boost::bind(&UDPReceiver::handle_receive_from, this,
                      boost::asio::placeholders::error,
                      boost::asio::placeholders::bytes_transferred,
                      cb_received));
    }
  }
};

class DeadlineTimer {
public:
  DeadlineTimer(boost::asio::io_service &io_service_,
                std::function<void()> cb_callback)
      : timer_(io_service_)
  {
  }

  void start(boost::posix_time::millisec ms)
  {
    timer_.expires_at(timer_.expires_at() + ms);
    timer_.async_wait(boost::bind(&DeadlineTimer::handle_timeout, this,
                                  boost::asio::placeholders::error));
  }

private:
  void handle_timeout(const boost::system::error_code &ec)
  {
    if (!ec) {
      cb_callback();
    }
  }

  boost::asio::deadline_timer timer_;
  std::function<void()> cb_callback;
};

class EventLoop {
public:
  auto create_udp_receiver(short port) -> std::shared_ptr<UDPReceiver>
  {
    return udp_receivers_.emplace_back(
        std::make_shared<UDPReceiver>(io_service_, port));
  }

  auto create_tcp_receiver(short port) -> std::shared_ptr<TCPReceiver>
  {
    return tcp_receivers_.emplace_back(
        std::make_shared<TCPReceiver>(io_service_, port));
  }

  auto create_deadline_timer(std::function<void()> cb_timeout)
  {
    return deadline_timers_.emplace_back(
        std::make_shared<DeadlineTimer>(io_service_, cb_timeout));
  }

  void await() { io_service_.run(); }

private:
  boost::asio::io_service io_service_;

  std::vector<std::shared_ptr<UDPReceiver>> udp_receivers_;
  std::vector<std::shared_ptr<TCPReceiver>> tcp_receivers_;

  std::vector<std::shared_ptr<DeadlineTimer>> deadline_timers_;
};

} // namespace event_loop
