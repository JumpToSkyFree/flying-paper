#include "peel/GLib/DateTime.h"
#include "peel/GLib/Date.h"
#include "peel/GLib/functions.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Widget.h"
#include <Widgets/DateTime.hpp>
#include <cstdint>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/class.h>

namespace FlyingPaper::Widgets {
PEEL_CLASS_IMPL(DateTime, "FlyingPaperDateTime", Gtk::Widget);
FloatPtr<DateTime> DateTime::create(String format) {
  return ::Object::create<DateTime>(prop_format(), format);
}
void DateTime::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<DateTime>();
}
inline void DateTime::vfunc_dispose() {
  if (datetime_label) {
    datetime_label->unparent();
    datetime_label = nullptr;
  }
  parent_vfunc_dispose<DateTime>();
}
inline void DateTime::init(Class *) {
  datetime_label = Gtk::Label::create("");
  if (format && has_timestamp) {
    String output = print_datetime();
    datetime_label->set_markup(output);
  }
  set_parent(datetime_label);
}
String DateTime::print_datetime() {
  RefPtr<GLib::DateTime> date =
      GLib::DateTime::create_from_unix_local(get_timestamp());
  String markup = "<span alpha='50%%'>%s</span>";
  return GLib::strdup_printf(markup.c_str(),
                             date->format(get_format()).c_str());
}
void DateTime::set_timestamp(std::int32_t timestamp) {
  if (datetime_label) {
    this->timestamp = timestamp;
    this->has_timestamp = true;
    String output = print_datetime();
    datetime_label->set_markup(output);
  }
}
void DateTime::set_format(String format) {
  this->format = format;
  if (datetime_label && has_timestamp) {
    String output = print_datetime();
    datetime_label->set_markup(output);
  }
}
void DateTime::set_date_text(String text) {
  if (datetime_label) {
    String markup = GLib::strdup_printf(
        "<span alpha='50%%'>%s</span>", text.c_str());
    datetime_label->set_markup(markup);
  }
}
String DateTime::get_date_text() const {
  if (datetime_label) {
    return datetime_label->get_text();
  }
  return "";
}
} // namespace FlyingPaper::Widgets
