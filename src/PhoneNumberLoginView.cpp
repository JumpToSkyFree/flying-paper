#include "glib.h"
#include "peel/Adw/StatusPage.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Gtk/Align.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/InputPurpose.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientable.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Widget.h"
#include <PhoneNumberLoginView.hpp>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/signal.h>

using namespace peel;

namespace FlyingPaper {
namespace Views {
PEEL_CLASS_IMPL(PhoneNumberInsertView, "FlyingPaperPhoneNumberInsertView",
                peel::Gtk::Widget);

inline void PhoneNumberInsertView::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<PhoneNumberInsertView>();
}
inline void PhoneNumberInsertView::vfunc_dispose() {
  if (status_page) {
    status_page->unparent();
    status_page = nullptr;
  }
  parent_vfunc_dispose<PhoneNumberLoginView>();
}

inline void PhoneNumberInsertView::init(Class *) {
  RefPtr<Gtk::Box> box = Gtk::Box::create(Gtk::Orientation::VERTICAL, 16);
  FloatPtr<Adw::StatusPage> _status_page = Adw::StatusPage::create();
  status_page = _status_page;

  input = Gtk::Entry::create();
  input->set_activates_default(true);
  input->set_input_purpose(Gtk::InputPurpose::EMAIL);
  input->set_placeholder_text("+1 (555) 555-0100");

  login_button = Gtk::Button::create();
  login_button->set_label("Next");
  login_button->connect_clicked(
      [this](Gtk::Button *) { g_print("%s", this->input->get_text()); });

  box->append(input);
  box->append(login_button);

  status_page->set_icon_name("input-dialpad-symbolic");
  status_page->set_title("Phone Number");
  status_page->set_description("Enter phone number to login.");
  status_page->set_child(box);
  status_page->set_size_request(250, -1);

  set_parent(std::move(status_page));
}

PEEL_CLASS_IMPL(PhoneNumberLoginView, "FlyingPaperPhoneNumberLoginView",
                Gtk::Widget);
inline void PhoneNumberLoginView::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<PhoneNumberLoginView>();
}
inline void PhoneNumberLoginView::vfunc_dispose() {
  if (view_stack) {
    view_stack->unparent();
    view_stack = nullptr;
  }
  parent_vfunc_dispose<PhoneNumberLoginView>();
}
inline void PhoneNumberLoginView::init(Class *) {
  FloatPtr<Adw::ViewStack> _view_stack = Adw::ViewStack::create();
  view_stack = _view_stack;
  set_halign(Gtk::Align::CENTER);
  set_valign(Gtk::Align::CENTER);
  FloatPtr<PhoneNumberInsertView> _insert_view =
      Object::create<PhoneNumberInsertView>();
  insert_view = _insert_view;
  view_stack->add(std::move(insert_view));
  set_parent(view_stack);
}

FloatPtr<PhoneNumberLoginView> PhoneNumberLoginView::create() {
  return Object::create<PhoneNumberLoginView>();
}
} // namespace Views
} // namespace FlyingPaper
