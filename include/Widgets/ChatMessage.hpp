#ifndef FLYING_PAPER_CHAT_MESSAGE_HPP
#define FLYING_PAPER_CHAT_MESSAGE_HPP

#include "peel/Gtk/Box.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/TextView.h"
#include "peel/Gtk/Viewport.h"
#include "peel/Gtk/Widget.h"
#include <cstdint>
#include <peel/FloatPtr.h>
#include <peel/Gtk/Gtk.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>

namespace FlyingPaper::Widgets {

class ChatMessage : public peel::Gtk::Widget {
  PEEL_SIMPLE_CLASS(ChatMessage, peel::Gtk::Widget)
  inline void init(Class *);
  inline void vfunc_dispose();

  peel::RefPtr<peel::Gtk::Box> container;
  peel::RefPtr<peel::Gtk::Box> bubble_container;
  peel::RefPtr<peel::Gtk::Box> message_container;

  td::td_api::message *message;
  td::td_api::MessageContent *message_content;

  bool is_group{false};

  peel::FloatPtr<peel::Gtk::TextView> create_label();
  peel::RefPtr<peel::Gtk::Box> create_message_container();

  std::int64_t message_id;

public:
  static peel::FloatPtr<ChatMessage> create();
  void set_message_content(td::td_api::message *message,
                           td::td_api::MessageContent *message_content);
  void set_is_group(bool);
  std::int64_t get_id();
  td::td_api::message *get_message_obj();
};

} // namespace FlyingPaper::Widgets

#endif
