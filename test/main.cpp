#include "EventLoop.hpp"

#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <chrono>
#include <functional>
#include <iostream>

static constexpr auto PORT{20000};

namespace tcp {

static std::vector<char> data;

void handle_received(std::vector<char> recv)
{
  std::cerr << "[TCP] received " << recv.size() << " bytes" << std::endl;
  data.insert(data.end(), recv.begin(), recv.end());
}

void handle_closed()
{
  std::cerr << "[TCP] connection closed, received a total of: " << data.size()
            << " bytes" << std::endl;
  data.clear();
}

} // namespace tcp

namespace udp {

static std::vector<char> data;

void handle_received(std::vector<char> recv,
                     std::shared_ptr<event_loop::DeadlineTimer> timer)
{
  std::cerr << "[UDP] received " << recv.size() << " bytes" << std::endl;
  data.insert(data.end(), recv.begin(), recv.end());

  timer->start(boost::posix_time::millisec(250));
}

void handle_timeout()
{
  std::cerr << "[UDP] data timeout, received a total of: " << data.size()
            << " bytes" << std::endl;
  data.clear();
}

} // namespace udp

int main(void)
{

  event_loop::EventLoop event_loop;
  event_loop.create_tcp_receiver(PORT)->accept(
      std::bind(tcp::handle_received, std::placeholders::_1),
      std::bind(tcp::handle_closed));

  auto timer = event_loop.create_deadline_timer(std::bind(udp::handle_timeout));

  event_loop.create_udp_receiver(PORT)->listen(
      std::bind(udp::handle_received, std::placeholders::_1, timer));

  event_loop.await();

  return 1;
}
