#ifndef FLYING_PAPER_DATETIME_HPP
#define FLYING_PAPER_DATETIME_HPP

#include "peel/Gtk/Label.h"
#include "peel/Gtk/Widget.h"
#include <climits>
#include <cstdint>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <peel/property.h>
using namespace peel;
namespace FlyingPaper::Widgets {
class DateTime final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(DateTime, Gtk::Widget)

  template <typename F> static void define_properties(F &f) {
    f.prop(prop_format(), "")
        .get(&DateTime::get_format)
        .set(&DateTime::set_format);
  }

  inline void vfunc_dispose();
  inline void init(Class *);

  std::int32_t timestamp{0};
  bool has_timestamp{false};
  String format;
  RefPtr<Gtk::Label> datetime_label;

  PEEL_PROPERTY(String, format, "format");

public:
  static FloatPtr<DateTime> create(String format);
  std::int32_t get_timestamp() const { return timestamp; }
  void set_timestamp(std::int32_t);
  String get_format() const { return format; }
  void set_format(String format);
  void set_date_text(String);
  String get_date_text() const;
  String print_datetime();
};
} // namespace FlyingPaper::Widgets

#endif
