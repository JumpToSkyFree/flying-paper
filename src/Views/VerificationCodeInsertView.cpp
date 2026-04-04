#include "peel/Adw/StatusPage.h"
#include "peel/GLib/DateTime.h"
#include "peel/GLib/functions.h"
#include "peel/Gdk/Display.h"
#include "peel/Gtk/Align.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/CallbackAction.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/InputPurpose.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/ShortcutTrigger.h"
#include "peel/Gtk/Widget.h"
#include <Views/VerificationCodeInsertView.hpp>
#include <peel/Adw/Adw.h>
#include <peel/FloatPtr.h>
#include <peel/GLib/Variant.h>
#include <peel/Gdk/Clipboard.h>
#include <peel/RefPtr.h>
#include <peel/UniquePtr.h>
#include <peel/class.h>

namespace FlyingPaper::Views {
PEEL_CLASS_IMPL(VerificationCodeInsertView,
                "FlyingPaperVerificationCodeInsertView", Gtk::Widget);
void VerificationCodeInsertView::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  sig_continue_clicked = Signal<VerificationCodeInsertView, void(void)>::create(
      "continue-clicked");
  override_vfunc_dispose<VerificationCodeInsertView>();
}
void VerificationCodeInsertView::vfunc_dispose() {
  if (status_page) {
    status_page->unparent();
    status_page = nullptr;
  }
  if (timeout_id) {
    // TODO: stop the countdown timer.
  }
  parent_vfunc_dispose<VerificationCodeInsertView>();
}
void VerificationCodeInsertView::handle_clipboard_paste() {
  // TODO: handle clipboard paste event.
  // auto clip = entry->get_clipboard();
  // clip->connect_signal("changed", Handler && handler)
}
void VerificationCodeInsertView::setup_entry(Gtk::Entry *entry) {
  entry->set_size_request(20, -1);
  // entry->set_tooltip_markup("");
  entry->set_max_length(1);
  entry->set_visible(true);
  entry->set_alignment(0.5);
  entry->set_input_purpose(Gtk::InputPurpose::DIGITS);
}
std::string VerificationCodeInsertView::get_verification_code() const {
  if (!entries.size())
    return "";
  std::string ret;
  for (auto &entry : entries) {
    ret += entry->get_text();
  }
  return ret;
}
void VerificationCodeInsertView::set_view_stack(Adw::ViewStack *view_stack) {
  this->view_stack = view_stack;
}
FloatPtr<Gtk::Button> VerificationCodeInsertView::resend_countdown_box() {
  // TODO: set the timing from response of tdlib.
  send_again = Gtk::Button::create();
  // FIX: countdown is not set correctly.
  RefPtr<GLib::DateTime> dt =
      GLib::DateTime::create_from_unix_utc(countdown / 1000);
  send_again->set_label(dt->format("%M:%S"));
  send_again->set_sensitive(false);
  send_again->add_css_class("flat");
  send_again->connect_signal("clicked", [](Gtk::Button *) {
    // TODO: request to resend the code.
  });
  timeout_id = GLib::timeout_add_seconds(1, [this]() {
    if (countdown <= 0) {
      send_again->set_label("Resend Code");
      send_again->set_sensitive(true);
      return false;
    }
    countdown -= 1000;
    RefPtr<GLib::DateTime> dt =
        GLib::DateTime::create_from_unix_utc(countdown / 1000);
    send_again->set_label(dt->format("%M:%S"));
    return true;
  });

  return send_again;
}
Signal<VerificationCodeInsertView, void(void)>
    VerificationCodeInsertView::sig_continue_clicked;
void VerificationCodeInsertView::init(Class *) {
  set_halign(Gtk::Align::CENTER);
  set_valign(Gtk::Align::CENTER);
  FloatPtr<Gtk::Box> box = Gtk::Box::create(Gtk::Orientation::VERTICAL, 16);
  inputs_box = Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 16);
  inputs_box->compute_expand(Gtk::Orientation::VERTICAL);
  inputs_box->set_visible(true);

  FloatPtr<Gdk::Display> display = Gdk::Display::get_default();
  clipboard = display->get_clipboard();

  FloatPtr<Adw::StatusPage> _status_page = Adw::StatusPage::create();
  status_page = _status_page;

  verify_button = Gtk::Button::create();
  verify_button->set_label("Continue");
  verify_button->connect_signal(
      "clicked", [this](Gtk::Button *) { sig_continue_clicked.emit(this); });

  FloatPtr<Gtk::Button> change_phone_number = Gtk::Button::create();
  change_phone_number->set_label("Go back");
  change_phone_number->add_css_class("flat");
  change_phone_number->connect_signal("clicked", [this](Gtk::Button *) {
    view_stack->set_visible_child_name("phone-number-insert");
  });

  box->append(inputs_box);
  box->append(verify_button);
  box->append(resend_countdown_box());
  box->append(std::move(change_phone_number));

  status_page->set_icon_name("padlock2-symbolic");
  status_page->set_title("Verification code");
  status_page->set_description("Enter the verification code to login.");
  status_page->set_child(box);
  status_page->set_size_request(250, -1);

  set_parent(std::move(_status_page));
}
FloatPtr<VerificationCodeInsertView> VerificationCodeInsertView::create() {
  FloatPtr<VerificationCodeInsertView> ptr =
      Object::create<VerificationCodeInsertView>();
  ptr->setup();
  return ptr;
}
void VerificationCodeInsertView::set_n_entries(std::uint32_t n_entries) {
  // TODO: temporary solution.
  entries.clear();

  for (std::uint32_t i = 0; i < n_entries; i++) {
    RefPtr<Gtk::Entry> entry = Gtk::Entry::create();
    setup_entry(entry);
    entries.push_back(entry);
    if (inputs_box)
      inputs_box->append(entry);
  }
}
void VerificationCodeInsertView::setup() {
  // TODO: set the client manager requests and handle responses here.
}
} // namespace FlyingPaper::Views
