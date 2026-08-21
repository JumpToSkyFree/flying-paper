#ifndef FLYING_PAPER_LAST_MESSAGE_HPP
#define FLYING_PAPER_LAST_MESSAGE_HPP

#include "peel/Gtk/Box.h"
#include "peel/Gtk/Image.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Widget.h"
#include <Telegram/ClientManager.hpp>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/property.h>
#include <td/telegram/td_api.h>
using namespace peel;
namespace FlyingPaper::Widgets {
class LastMessage final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(LastMessage, Gtk::Widget)
  inline void init(Class *);
  inline void vfunc_dispose();
  template <typename F> static void define_properties(F &f) {
    f.prop(prop_read(), true)
        .get(&LastMessage::get_read)
        .set(&LastMessage::set_read);
  }

  Gtk::Box *container;
  RefPtr<Gtk::Label> text_content;
  RefPtr<Gtk::Image> image_content;
  bool read{true};

  void set_label_with_markup(bool read, const std::string &text);

  PEEL_PROPERTY(bool, read, "read");

public:
  static RefPtr<LastMessage> create();
  void set_message_content(bool read,
                           td::td_api::object_ptr<td::td_api::MessageContent>);
  bool get_read() const { return read; }
  void set_read(bool read);
};
} // namespace FlyingPaper::Widgets

#endif
