#include "EventLoop.hpp"

#include <functional>
#include <iostream>

static constexpr auto PORT{20000};

static std::vector<char> data;

void handle_received(std::vector<char> recv)
{
  std::cerr << "received " << recv.size() << " bytes" << std::endl;
  data.insert(data.end(), recv.begin(), recv.end());
}

void handle_closed()
{
  std::cerr << "connection closed, received a total of: " << data.size()
            << " bytes" << std::endl;
  data.clear();
}

int main(void)
{
  EventLoop event_loop{PORT};
  event_loop.accept(std::bind(handle_received, std::placeholders::_1),
                    std::bind(handle_closed));
  event_loop.await();

  return 1;
}
