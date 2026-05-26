#ifndef FLYING_PAPER_CHAT_MESSAGE_INPUT_HPP
#define FLYING_PAPER_CHAT_MESSAGE_INPUT_HPP

#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/TextBuffer.h"
#include "peel/Gtk/TextView.h"
#include "peel/Gtk/Widget.h"
#include <cstdint>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

namespace FlyingPaper::Widgets {
class MessageInput : public peel::Gtk::Widget {
  PEEL_SIMPLE_CLASS(MessageInput, peel::Gtk::Widget)

  inline void init(Class *);
  inline void vfunc_dispose();

  peel::RefPtr<peel::Gtk::Box> container;
  peel::RefPtr<peel::Gtk::Entry> entry;
  peel::RefPtr<peel::Gtk::Button> send_button;
  peel::Gtk::TextBuffer *text_buffer;
  std::int64_t chat_id;

public:
  static peel::FloatPtr<MessageInput> create();
  void set_chat_id(std::int64_t);
};
} // namespace FlyingPaper::Widgets

#endif // FLYING_PAPER_CHAT_MESSAGE_INPUT_HPP
