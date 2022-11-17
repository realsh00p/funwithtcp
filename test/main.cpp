#include "EventLoop.hpp"

#include <iostream>

static constexpr auto PORT{20000};

int main(void) {

  EventLoop event_loop{PORT};
  event_loop.await();

  return 1;
}
