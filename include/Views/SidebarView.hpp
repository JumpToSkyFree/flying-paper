#ifndef FLYING_PAPER_SIDEBAR_VIEW_HPP
#define FLYING_PAPER_SIDEBAR_VIEW_HPP

#include "Widgets/Avatar.hpp"
#include "Widgets/ChatListItem.hpp"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/GestureClick.h"
#include "peel/Gtk/MenuButton.h"
#include "peel/Gtk/Widget.h"
#include <cstdint>
#include <functional>
#include <peel/FloatPtr.h>
#include <peel/Gtk/Box.h>
#include <peel/RefPtr.h>
#include <peel/WeakPtr.h>
#include <peel/class.h>
#include <peel/signal.h>
#include <td/telegram/td_api.h>
#include <unordered_map>

using namespace peel;

namespace FlyingPaper::Views {
class Sidebar final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(Sidebar, Gtk::Widget)
public:
  using Callback = std::function<void(Widgets::ChatListItem *)>;

private:
  friend Gtk::Widget;

  inline void init(Class *);
  inline void vfunc_dispose();

  Gtk::Box *container{nullptr};
  RefPtr<Adw::ToastOverlay> toast_overlay;
  RefPtr<Gtk::GestureClick> avatar_click_controller;
  RefPtr<Widgets::Avatar> avatar;
  std::uint64_t file_update_subscription_id{0};
  Callback cb;

  FloatPtr<Adw::HeaderBar> make_header_bar();
  FloatPtr<Gtk::MenuButton> make_menu();
  void make_profile_avatar();
  td::td_api::object_ptr<td::td_api::ChatList> chat_list{nullptr};

  std::unordered_map<std::int64_t, RefPtr<Widgets::ChatListItem>> chats_map;
  void set_avatar_fullname(const td::td_api::user &);
  void set_avatar_image(const td::td_api::file &);

  Widgets::ChatListItem *find_chat_by_id(std::int64_t);
  Widgets::ChatListItem *active_item{nullptr};
  RefPtr<Gtk::Box> chats_box;

  std::uint64_t update_new_chat_subscription{0};
  std::uint64_t update_chat_position{0};
  std::uint64_t update_new_chat{0};
  std::uint64_t update_chat_last_message{0};
  std::uint64_t update_chat_read_inbox{0};

  void order_chat_item_position(Widgets::ChatListItem *item);

  static peel::Signal<Sidebar, void()> on_click;

public:
  static FloatPtr<Sidebar> create();
  void set_on_chat_item_selected(Callback);
};
} // namespace FlyingPaper::Views

#endif
