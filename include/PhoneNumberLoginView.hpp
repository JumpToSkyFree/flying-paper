#ifndef FLYING_PAPER_LOGIN_WITH_PHONE_NUMBER_VIEW_HPP
#define FLYING_PAPER_LOGIN_WITH_PHONE_NUMBER_VIEW_HPP
#include "peel/Adw/StatusPage.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/GestureClick.h"
#include "peel/Gtk/Widget.h"
#include <peel/FloatPtr.h>
#include <peel/Gtk/Gtk.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/signal.h>
namespace FlyingPaper {
namespace Views {

// TODO: Make a seperate better looking phone number input that have two
// inputs to pick country code and insert phone number.
class PhoneNumberInsertView final : public peel::Gtk::Widget {
  PEEL_SIMPLE_CLASS(PhoneNumberInsertView, peel::Gtk::Widget);
  friend peel::Gtk::Widget;

  inline void init(Class *);
  inline void vfunc_dispose();

  peel::Adw::StatusPage *status_page;
  peel::RefPtr<peel::Gtk::Entry> input;
  peel::RefPtr<peel::Gtk::Button> login_button;

  void on_released(peel::Gtk::GestureClick *, int n_press, double x, double y);

  peel::String phone_number;

  template <typename F> static void define_properties(F &f) {
    f.prop(prop_phone_number(), nullptr)
        .get(&PhoneNumberInsertView::get_phone_number);
  }

public:
  const char *get_phone_number() { return phone_number; };

  PEEL_PROPERTY(const char *, phone_number, "phone-number");
};

class PhoneNumberLoginView final : public peel::Gtk::Widget {
  PEEL_SIMPLE_CLASS(PhoneNumberLoginView, peel::Gtk::Widget);
  friend peel::Gtk::Widget;

  PhoneNumberInsertView *insert_view;
  peel::Adw::ViewStack *view_stack;

  inline void init(Class *);
  inline void vfunc_dispose();

public:
  static peel::FloatPtr<PhoneNumberLoginView> create();
};
} // namespace Views
} // namespace FlyingPaper

#endif
