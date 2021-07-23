#include "boost/sml.hpp"
#include <cassert>
#include <cstdio>
#include <string>
#include <iostream>
#include <unordered_map>
#include <set>

namespace sml = boost::sml;

template <class T>
void dump_transition() noexcept
{
    auto src_state = std::string{sml::aux::string<typename T::src_state>{}.c_str()};
    auto dst_state = std::string{sml::aux::string<typename T::dst_state>{}.c_str()};
    if (dst_state == "X")
    {
        dst_state = "[*]";
    }

    if (T::initial)
    {
        std::cout << "[*] --> " << src_state << std::endl;
    }

    const auto has_event = !sml::aux::is_same<typename T::event, sml::anonymous>::value;
    const auto has_guard = !sml::aux::is_same<typename T::guard, sml::front::always>::value;
    const auto has_action = !sml::aux::is_same<typename T::action, sml::front::none>::value;

    const auto is_entry = sml::aux::is_same<typename T::event, sml::back::on_entry<sml::_, sml::_>>::value;
    const auto is_exit = sml::aux::is_same<typename T::event, sml::back::on_exit<sml::_, sml::_>>::value;

    if (is_entry || is_exit)
    {
        std::cout << src_state;
    }
    else
    { // state to state transition
        std::cout << src_state << " --> " << dst_state;
    }

    if (has_event || has_guard || has_action)
    {
        std::cout << " :";
    }

    if (has_event)
    {
        auto event = std::string(boost::sml::aux::get_type_name<typename T::event>());
        if (is_entry)
        {
            event = "entry";
        }
        else if (is_exit)
        {
            event = "exit";
        }
        std::cout << " " << event;
    }

    if (has_guard)
    {
        std::cout << " [" << boost::sml::aux::get_type_name<typename T::guard::type>() << "]";
    }

    if (has_action)
    {
        std::cout << " / " << boost::sml::aux::get_type_name<typename T::action::type>();
    }

    std::cout << std::endl;
}

template <template <class...> class T, class... Ts>
void dump_transitions(const T<Ts...> &) noexcept
{
    int _[]{0, (dump_transition<Ts>(), 0)...};
    (void)_;
}

template <class SM>
void dump(const SM &) noexcept
{
    std::cout << "@startuml" << std::endl
              << std::endl;
    dump_transitions(typename SM::transitions{});
    std::cout << std::endl
              << "@enduml" << std::endl;
}

// CurrentUser
struct Context
{
    int id;
    int page;
    std::string info_text;
    std::string yn_text;
    std::string err_text;
};

enum DPErrorCode
{
    PLCErr,
    WatchdogErr,
    RFIDErr,
    ParkOutTimeout,
    ParkInTimeout,
    DBErr
};

// Event
struct e_Timeout
{
};
struct e_Error
{
    std::set<DPErrorCode> errs;
};
struct e_ClearError
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
    std::cout << "[yN] " << ctx.yn_text << std::endl;
}

enum ParkAction
{
    In_WaitUser,
    In,
    Out
};

bool q_has_error = false;
bool q_can_parkIn = true;
bool q_can_parkOut = true;
bool q_can_parkIn_Final = true;
bool q_is_inQueue = true;
std::unordered_map<int, ParkAction> q_orders;

void order_finished()
{
    q_orders = std::unordered_map<int, ParkAction>(++q_orders.begin(), q_orders.end());
}

// Guard
struct G_Has_Order
{
    auto operator()()
    {
        return !q_orders.empty();
    };
} g_has_order;

struct G_Has_Err
{
    auto operator()()
    {
        return q_has_error;
    };
} g_has_error;

struct G_Can_ParkIn
{
    auto operator()(const e_CardDetect &event)
    {
        return q_can_parkIn && q_orders.find(event.id) != q_orders.end() && q_orders[event.id] == ParkAction::In_WaitUser;
    };
} g_can_parkIn;
struct G_Can_ParkOut
{
    auto operator()()
    {
        return q_can_parkOut;
    };
} g_can_parkOut;

struct G_Can_ParkIn_Final
{
    auto operator()()
    {
        return q_can_parkIn_Final;
    };
} g_can_parkIn_Final;

struct G_Is_InQueue
{
    auto operator()(const e_CardDetect &event)
    {
        return q_orders.find(event.id) != q_orders.end();
    };
} g_is_inQueue;

struct G_WindshieldInOrder
{
    auto operator()(const e_CardDetect &event)
    {
        return q_orders.find(event.id) != q_orders.end() && q_orders[event.id] == ParkAction::In_WaitUser;
    };
} g_windshield_in_order;
struct G_Cancel_ParkOut
{
    auto operator()(const e_CardDetect &event)
    {
        return q_orders.find(event.id) != q_orders.end() && q_orders[event.id] == ParkAction::Out;
    };
} g_cancel_parkOut;

// Transition
constexpr auto reset_user = [](Context &ctx)
{
    std::cout << "clear User (" << ctx.id << ")" << std::endl;
    ctx.id = 0;
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

constexpr auto set_uid = [](Context &ctx, const auto &event)
{
    ctx.id = event.id;
};

constexpr auto cancel_parkin_text = [](Context &ctx, const auto &event)
{
    ctx.id = event.id;
    set_yn_text(ctx, "Cancel Parkin");
};

constexpr auto cancel_parkout_text = [](Context &ctx, const auto &event)
{
    ctx.id = event.id;
    set_yn_text(ctx, "Cancel Parkout");
};

constexpr auto remove_order = [](Context &ctx)
{
    q_orders.erase(ctx.id);
    std::cout << "#Orders after removed: " << q_orders.size() << std::endl;
    std::cout << "Remove Order with userId: " << ctx.id << std::endl;
    ctx.id = 0;
};

constexpr auto set_info_yNconfirmed_text = [](Context &ctx)
{
    ctx.info_text = "[Info] Your order is successfully canceled!\n";
    std::cout << ctx.info_text << "\n";
};
constexpr auto set_info_yNreject_text = [](Context &ctx)
{
    ctx.info_text = "[Info] Your order is not canceled!\n";
    std::cout << ctx.info_text << "\n";
};

constexpr auto set_err_text = [](Context &ctx, const e_Error &event)
{
    for (const auto &e : event.errs)
    {
        std::string msg;
        switch (e)
        {
        case DPErrorCode::ParkInTimeout:
            msg += "[Error] ParkIn timeout";
            break;
        case DPErrorCode::ParkOutTimeout:
            msg += "[Error] ParkOut timeout";
            break;
        case DPErrorCode::PLCErr:
            msg += "[Error] PLCErr";
            break;
        case DPErrorCode::RFIDErr:
            msg += "[Error] RFIDErr";
            break;
        case DPErrorCode::WatchdogErr:
            msg += "[Error] WatchdogErr";
            break;
        case DPErrorCode::DBErr:
            msg += "[Error] DBErr";
            break;
        }

        ctx.err_text.append(msg);
    }
};

constexpr auto clear_err = [](Context &ctx)
{
    ctx.err_text = "";
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

    // [ToDo] User parkIn cancel --> if Car is still in Room?
        return make_transition_table(
            *"idle"_s + event<e_Wakeup> / update_queue = "status"_s,
            *"idle"_s + event<e_Error> / (update_queue, set_err_text) = "status"_s,
            "status"_s + event<e_Timeout>[!(g_has_order || g_has_error)] = "idle"_s,
            "status"_s + event<e_Timeout>[g_has_order] / update_queue = "status"_s,
            "status"_s + event<e_Timeout>[g_has_error] = "error"_s,
            "status"_s + event<e_CardDetect>[g_windshield_in_order] / set_uid = "parkIn"_s,
            "status"_s + event<e_CardDetect>[g_cancel_parkOut] / cancel_parkout_text = "yn"_s,
            "status"_s + event<e_CardDetect>[g_can_parkOut] / (parkout_success, [](const auto &event)
                                                               { q_orders[event.id] = ParkAction::Out; }) = "info"_s,
            "status"_s + event<e_CardDetect>[!g_can_parkOut] / parkout_failed = "info"_s,
            "parkIn"_s + event<e_CardDetect>[g_can_parkIn_Final] / parkin_success = "info"_s,
            "parkIn"_s + event<e_CardDetect>[!g_can_parkIn_Final] / parkin_failed = "info"_s,
            "parkIn"_s + event<e_Timeout> / reset_user = "status"_s,
            "info"_s + event<e_InfoConfirmed> / reset_user = "status"_s,
            "info"_s + event<e_Timeout> / reset_user = "status"_s,
            "yn"_s + event<e_ynConfirm> / remove_order = "info"_s,
            "yn"_s + event<e_ynReject> / reset_user = "info"_s,
            "yn"_s + event<e_Timeout> / reset_user = "status"_s,
            "error"_s + event<e_ClearError> / clear_err = "status"_s);
    }
};

// g++ -std=c++17 -O2 -fno-exceptions -Wall -Wextra -Werror -pedantic gui_sm.cpp
int main()
{
    using namespace sml;

    // [idle-->] Start at idle page
    Context ctx{0, 0, "idle", "yn", {}};
    sm<GUI_SM> sm{ctx}; // pass dependencies via ctor
    assert(sm.is("idle"_s));

    // [-->status] User wake up Touchscreen
    sm.process_event(e_Wakeup{});
    assert(sm.is("status"_s));

    // [-->idle] Timeout goto sleep
    sm.process_event(e_Timeout{});
    assert(sm.is("idle"_s));

    // <User Story 1>Parkout Success
    // <U1.1> Parkout successfully and timeout at info page
    // [-->status-->info] Wake up and use parkout
    sm.process_event(e_Wakeup{});
    sm.process_event(e_CardDetect{2});
    q_orders[2] = ParkAction::Out;
    assert(sm.is("info"_s));
    sm.process_event(e_Timeout{});
    assert(sm.is("status"_s));

    ctx.id = 2;
    remove_order(ctx);
    // <U1.2> Parkout successfully and user confirms on info page
    // [-->info-->status]
    sm.process_event(e_CardDetect{2});
    assert(q_orders[2] == ParkAction::Out);
    assert(sm.is("info"_s));
    sm.process_event(e_InfoConfirmed{});
    assert(sm.is("status"_s));
    assert(q_orders.size() == 1);

    // <U1.3> User confirm cancel its parkout order
    // [-->yN-->info-->status]
    sm.process_event(e_CardDetect{2});
    assert(sm.is("yn"_s));
    sm.process_event(e_ynConfirm{});
    assert(sm.is("info"_s));
    sm.process_event(e_InfoConfirmed{});
    assert(sm.is("status"_s));

    // <U1.4> User reject cancel its parkout order
    // [-->yN-->info-->status]
    sm.process_event(e_CardDetect{2});
    sm.process_event(e_InfoConfirmed{});
    sm.process_event(e_CardDetect{2});
    assert(sm.is("yn"_s));
    sm.process_event(e_ynReject{});
    assert(sm.is("info"_s));
    sm.process_event(e_InfoConfirmed{});
    assert(sm.is("status"_s));
    assert(q_orders[2] == ParkAction::Out);

    // <U1.5> User reject cancel its parkout order
    // [-->yN-->status]
    sm.process_event(e_CardDetect{2});
    assert(sm.is("yn"_s));
    sm.process_event(e_Timeout{});
    assert(sm.is("status"_s));
    assert(q_orders[2] == ParkAction::Out);

    ctx.id = 2;
    remove_order(ctx);

    // <User Story 2>ParkIn Success
    // <U2.1> ParkIn but not confirm at parkIn page
    // [-->parkIn-->info-->status] Wake up and use parkout
    q_orders[2] = ParkAction::In_WaitUser;
    sm.process_event(e_CardDetect{2});
    assert(sm.is("parkIn"_s));
    sm.process_event(e_Timeout{});
    assert(sm.is("status"_s));
    assert(q_orders[2] == ParkAction::In_WaitUser);

    // dump(sm);

    return 0;
}