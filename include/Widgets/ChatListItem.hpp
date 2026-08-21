#ifndef FLYING_PAPER_CHAT_LIST_ITEM_HPP
#define FLYING_PAPER_CHAT_LIST_ITEM_HPP

#include "Widgets/Avatar.hpp"
#include "Widgets/DateTime.hpp"
#include "Widgets/LastMessage.hpp"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/ToggleButton.h"
#include "peel/Gtk/Widget.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/signal.h>
#include <td/telegram/td_api.h>

using namespace peel;

namespace FlyingPaper::Widgets {
class ChatListItem final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(ChatListItem, Gtk::Widget)
  inline void init(Class *);
  inline void vfunc_dispose();

  Gtk::Box *container{nullptr};
  RefPtr<Widgets::Avatar> avatar;

  RefPtr<Adw::ToastOverlay> sidebar_toast_overlay;
  RefPtr<Gtk::Label> chat_title_label;
  RefPtr<Widgets::DateTime> datetime;
  RefPtr<Gtk::Box> chat_title_and_last_message_time_container;
  RefPtr<Gtk::Box> last_chat_sent_content_and_unread_nmessages_count;
  RefPtr<Gtk::Box> info_container;
  RefPtr<Widgets::LastMessage> last_message;
  RefPtr<Gtk::ToggleButton> button;

  void set_avatar_text(const std::string &fullname);
  void
  set_avatar_picture(const td::td_api::object_ptr<td::td_api::file> &photo);

  FloatPtr<Gtk::Box> make_vertical_container(FloatPtr<Gtk::Widget> left,
                                             FloatPtr<Gtk::Widget> right);
  FloatPtr<Gtk::Box> make_horizontal_container(FloatPtr<Gtk::Widget> left,
                                               FloatPtr<Gtk::Widget> right);

  String chat_title;
  std::int32_t timestamp{0};
  FloatPtr<Gtk::Widget> chat_content;
  String username;
  std::int64_t chat_id{0};
  std::int64_t order{0};

  static Signal<ChatListItem, void(void)> sig_on_click;
  PEEL_SIGNAL_CONNECT_METHOD(chat_list_item_clicked, sig_on_click);

public:
  static FloatPtr<ChatListItem> create();

  peel::String get_chat_title() const { return chat_title; }
  void set_chat_title(String);
  Gtk::Widget *get_chat_content() const { return chat_content; }
  void set_chat_content(FloatPtr<Gtk::Widget>);
  std::int32_t get_timestamp() const { return timestamp; }
  void set_timestamp(std::int32_t);
  String get_username() const { return username; }
  void set_username(String);
  std::int64_t get_chat_id() const { return chat_id; }
  void set_chat_id(std::int64_t);
  std::int64_t get_order() const { return order; }
  void set_order(std::int64_t order);
  RefPtr<Gtk::ToggleButton> get_button();

  void set_chat(const std::shared_ptr<td::td_api::chat> &chat);
  void set_new_message(const std::shared_ptr<td::td_api::updateNewMessage> &);
  void set_last_message(const std::shared_ptr<td::td_api::message> &);
  RefPtr<Widgets::LastMessage> get_last_message();
  // void set_on_click(std::function<void()>);
};
} // namespace FlyingPaper::Widgets

#endif
