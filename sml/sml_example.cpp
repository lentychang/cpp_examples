#include "boost/sml.hpp"
#include <cassert>
#include <cstdio>
namespace sml = boost::sml;

struct sender {
  template<class TMsg>
  constexpr void send(const TMsg& msg) { std::printf("send: %d\n", msg.id); }
};

struct ack { bool valid{}; };
struct fin { int id{}; bool valid{}; };
struct release {};
struct timeout {};

// Guard
constexpr auto is_valid = [](const auto& event) { return event.valid; };
// Transition
constexpr auto send_fin = [](sender& s) { s.send(fin{0}); };
constexpr auto send_ack = [](sender& s, const auto& event) { s.send(event); };

struct tcp_release {
  auto operator()() const {
    using namespace sml;
    /**
     * Initial state: *initial_state
     * Transition DSL: src_state + event [ guard ] / action = dst_state
     */
    return make_transition_table(
      *"established"_s + event<release>          / send_fin  = "fin wait 1"_s,
       "fin wait 1"_s  + event<ack> [ is_valid ]             = "fin wait 2"_s,
       "fin wait 2"_s  + event<fin> [ is_valid ] / send_ack  = "timed wait"_s,
       "timed wait"_s  + event<timeout>                      = X
    );
  }
};

// compile g++ -std=c++17 -O2 -fno-exceptions -Wall -Wextra -Werror -pedantic sml_sm.cpp
int main() {
  using namespace sml;

  sender s{};
  sm<tcp_release> sm{s}; // pass dependencies via ctor
  assert(sm.is("established"_s));

  sm.process_event(release{}); // complexity O(1)
  assert(sm.is("fin wait 1"_s));

  sm.process_event(ack{true}); // prints 'send: 0'
  assert(sm.is("fin wait 2"_s));

  sm.process_event(fin{42, true}); // prints 'send: 42'
  assert(sm.is("timed wait"_s));

  sm.process_event(timeout{});
  assert(sm.is(X));  // terminated
}