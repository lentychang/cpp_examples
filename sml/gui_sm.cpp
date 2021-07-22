#include "boost/sml.hpp"
#include <cassert>
#include <cstdio>
#include <string>
#include <iostream>
#include <unordered_map>

namespace sml = boost::sml;
// CurrentUser
struct Context
{
    int id;
    int page;
    std::string info_text;
    std::string yn_text;
};

// Event
struct e_Timeout
{
};
struct e_Wakeup
{
};
struct e_CardDetect
{
    int id;
};
struct e_WindshieldDetect
{
    int id;
};
struct e_InfoConfirmed
{
};
struct e_ynReject
{
};
struct e_ynConfirm
{
};
// struct ParkInFinalConfirm{};

void set_info_text(Context &ctx, const std::string &text)
{
    ctx.info_text = text;
    std::cout << "[Info] " << ctx.info_text << std::endl;
}

void set_yn_text(Context &ctx, const std::string &text)
{
    ctx.yn_text = text;
    std::cout << "[yN] " << ctx.info_text << std::endl;
}

enum ParkAction
{
    IN,
    OUT
};

bool q_has_error = false;
bool q_can_parkIn = true;
bool q_can_parkOut = true;
bool q_can_parkIn_Final = true;
bool q_is_inQueue = true;
std::unordered_map<int, ParkAction> q_orders;

void order_finished(){
    q_orders = std::unordered_map<int, ParkAction>(++q_orders.begin(),q_orders.end());
}

// Guard
constexpr auto g_has_order = []()
{ return !q_orders.empty(); };
constexpr auto g_has_error = []()
{ return q_has_error; };
constexpr auto g_can_parkIn = []()
{ return q_can_parkIn; };
constexpr auto g_can_parkOut = []()
{ return q_can_parkOut; };
constexpr auto g_can_parkIn_Final = []()
{ return q_can_parkIn_Final; };
constexpr auto g_is_inQueue = [](const auto &event)
{ return q_orders.find(event.id) != q_orders.end();};

constexpr auto g_cancel_parkIn = [](const auto &event)
{
    return q_orders[event.id] == ParkAction::IN;
};
constexpr auto g_cancel_parkOut = [](const auto &event)
{
    return q_orders[event.id] == ParkAction::OUT;
};

// Transition
constexpr auto reset_user = [](Context &ctx)
{
    ctx.id = 0;
    std::cout << "reset User" << std::endl;
};

constexpr auto update_queue = [](Context &ctx)
{
    ctx.info_text = "update_queue";
    std::cout << ctx.info_text << std::endl;
};

constexpr auto parkout_success = [](Context &ctx)
{
    set_info_text(ctx, "ParkOut success");
};

constexpr auto parkout_failed = [](Context &ctx)
{
    set_info_text(ctx, "ParkOut failed");
};

constexpr auto parkout_timeout = [](Context &ctx)
{
    set_info_text(ctx, "ParkOut timeout");
};

constexpr auto parkin_success = [](Context &ctx)
{
    set_info_text(ctx, "ParkIn success");
};

constexpr auto parkin_failed = [](Context &ctx)
{
    set_info_text(ctx, "ParkIn failed");
};

constexpr auto parkin_timeout = [](Context &ctx)
{
    set_info_text(ctx, "ParkIn Timeout");
};

constexpr auto cancel_parkin_text = [](Context &ctx)
{
    set_yn_text(ctx, "Cancel Parkin");
};

constexpr auto cancel_parkout_text = [](Context &ctx,const auto& event)
{
    ctx.id = event.id;
    set_yn_text(ctx, "Cancel Parkout");
};

constexpr auto remove_order = [](Context &ctx)
{
    q_orders.erase(ctx.id);
    std::cout <<"#Orders after removed: " << q_orders.size() << std::endl;
    std::cout << "Remove Order with userId: " << ctx.id << std::endl;;
    ctx.id = 0;
};

struct GUI_SM
{
    auto operator()() const
    {
        using namespace sml;
        /**
     * Initial state: *initial_state
     * Transition DSL: src_state + event [ guard ] / action = dst_state
     */
        return make_transition_table(
            *"idle"_s + event<e_Wakeup> / update_queue = "status"_s,
            "status"_s + event<e_Timeout>[!(g_has_order || g_has_error)] = "idle"_s,
            "status"_s + event<e_Timeout>[g_has_order] / update_queue = "status"_s,
            "status"_s + event<e_Timeout>[g_has_error] = "error"_s,
            "status"_s + event<e_CardDetect>[g_is_inQueue && g_cancel_parkIn] / cancel_parkin_text = "yn"_s,
            "status"_s + event<e_CardDetect>[g_is_inQueue && g_cancel_parkOut] / (cancel_parkout_text,remove_order ) = "yn"_s,
            "status"_s + event<e_CardDetect>[!g_is_inQueue && g_can_parkOut] / (parkout_success, [](const auto& event){q_orders[event.id] = ParkAction::OUT;}) = "info"_s,
            "status"_s + event<e_CardDetect>[!g_is_inQueue && !g_can_parkOut] / parkout_failed = "info"_s,
            "status"_s + event<e_CardDetect>[!g_is_inQueue && g_can_parkIn]  = "parkIn"_s,
            "status"_s + event<e_CardDetect>[!g_is_inQueue && !g_can_parkIn] / parkin_failed = "info"_s,
            // "status"_s + event<CardDetect>[!g_can_parkOut] / parkout_timeout = "info"_s,
            // "status"_s + event<e_WindshieldDetect>[g_can_parkIn] = "parkIn"_s,
            "parkIn"_s + event<e_CardDetect>[g_can_parkIn_Final] / parkin_success = "info"_s,
            "parkIn"_s + event<e_CardDetect>[!g_can_parkIn_Final] / parkin_failed = "info"_s,
            "parkIn"_s + event<e_Timeout> / (parkin_timeout, reset_user) = "info"_s,
            "info"_s + event<e_InfoConfirmed> / reset_user = "status"_s,
            "info"_s + event<e_Timeout> / reset_user = "status"_s,
            "yn"_s + event<e_ynConfirm> / reset_user = "status"_s,
            "yn"_s + event<e_ynReject> / reset_user = "status"_s,
            "yn"_s + event<e_Timeout> / reset_user = "status"_s,
            "error"_s = X);
    }
};

// g++ -std=c++17 -O2 -fno-exceptions -Wall -Wextra -Werror -pedantic gui_sm.cpp
int main()
{
    using namespace sml;

    // Start at idle page
    Context ctx{0, 0, "idle","yn"};
    sm<GUI_SM> sm{ctx}; // pass dependencies via ctor
    assert(sm.is("idle"_s));

    // User wake up Touchscreen
    sm.process_event(e_Wakeup{});
    assert(sm.is("status"_s));    
    // Timeout goto sleep
    sm.process_event(e_Timeout{});
    assert(sm.is("idle"_s));

    // Wake up and use parkout
    sm.process_event(e_Wakeup{});
    sm.process_event(e_CardDetect{2});
    q_orders[2] = ParkAction::OUT;
    assert(sm.is("info"_s));
    
    sm.process_event(e_Timeout{});
    assert(sm.is("status"_s));

    sm.process_event(e_Timeout{});    
    assert(sm.is("status"_s));

    sm.process_event(e_Wakeup{});
    sm.process_event(e_CardDetect{2});
    q_orders[2] = ParkAction::OUT;
    assert(sm.is("yn"_s));

    return 0;
}