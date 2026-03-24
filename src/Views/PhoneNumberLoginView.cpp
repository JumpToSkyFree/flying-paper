#include "Telegram/ClientManager.hpp"
#include "peel/Adw/StatusPage.h"
#include "peel/Adw/Toast.h"
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
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <Views/PhoneNumberLoginView.hpp>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/signal.h>
#include <td/telegram/td_api.h>

using namespace peel;

namespace FlyingPaper {
namespace Views {
PEEL_CLASS_IMPL(PhoneNumberInsertView, "FlyingPaperPhoneNumberInsertView",
                peel::Gtk::Widget);

inline void PhoneNumberInsertView::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());

  sig_next_clicked =
      Signal<PhoneNumberInsertView, void(void)>::create("next-clicked");
  override_vfunc_dispose<PhoneNumberInsertView>();
}
inline void PhoneNumberInsertView::vfunc_dispose() {
  if (status_page) {
    status_page->unparent();
    status_page = nullptr;
  }
  parent_vfunc_dispose<PhoneNumberInsertView>();
}

inline void PhoneNumberInsertView::init(Class *) {
  RefPtr<Gtk::Box> box = Gtk::Box::create(Gtk::Orientation::VERTICAL, 16);
  FloatPtr<Adw::StatusPage> _status_page = Adw::StatusPage::create();
  status_page = _status_page;

  input = Gtk::Entry::create();
  input->set_activates_default(true);
  input->set_input_purpose(Gtk::InputPurpose::PHONE);
  input->set_placeholder_text("+1 (555) 555-0100");

  login_button = Gtk::Button::create();
  login_button->set_label("Next");
  login_button->connect_signal("clicked", [this](Gtk::Button *button) {
    (void)button;
    this->sig_next_clicked.emit(this);
  });

  box->append(input);
  box->append(login_button);

  status_page->set_icon_name("input-dialpad-symbolic");
  status_page->set_title("Phone Number");
  status_page->set_description("Enter phone number to login.");
  status_page->set_child(box);
  status_page->set_size_request(250, -1);

  set_parent(std::move(status_page));
}
const char *PhoneNumberInsertView::get_phone_number() const {
  return this->input->get_text();
}
peel::Signal<PhoneNumberInsertView, void(void)>
    PhoneNumberInsertView::sig_next_clicked;
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
  insert_view = Object::create<PhoneNumberInsertView>();
  view_stack->add_named(insert_view, "phone-number-insert");
  insert_view->connect_signal("next-clicked", [this](PhoneNumberInsertView *) {
    // TODO: Send phone number informations and receve code to login.
    Telegram::ClientManagerAccessor::send(
        client_manager,
        td::td_api::make_object<td::td_api::setAuthenticationPhoneNumber>(
            insert_view->get_phone_number(), nullptr),
        [this](const Telegram::ClientManager::SharedObject &obj) {
          this->handle_authentication_failed(obj);
        });
  });
  set_parent(view_stack);
}
void PhoneNumberLoginView::setup() {
  Telegram::ClientManagerAccessor::subscribe(
      client_manager, td::td_api::authorizationStateWaitCode::ID,
      [](const Telegram::ClientManager::SharedObject &obj) {
        const auto &object = Telegram::ClientManager::cast<
            td::td_api::authorizationStateWaitCode>(obj);
        // TODO: When the phone is valid, view stach must activate a
        // verification code insert view with a timeout click.
      });
}
void PhoneNumberLoginView::handle_authentication_failed(
    const Telegram::ClientManager::SharedObject &obj) {
  if (obj->get_id() == td::td_api::error::ID) {
    const auto &error = Telegram::ClientManager::cast<td::td_api::error>(obj);
    if (error.message_ == "PHONE_NUMBER_INVALID") {
      RefPtr<Adw::Toast> toast = Adw::Toast::create("Invalid phone number");
      this->toast_overlay->dismiss_all();
      this->toast_overlay->add_toast(toast);
    }
  }
}
FloatPtr<PhoneNumberLoginView> PhoneNumberLoginView::create(
    std::shared_ptr<Telegram::ClientManager> client_manager,
    peel::RefPtr<peel::Adw::ToastOverlay> toast_overlay) {
  auto ptr = Object::create<PhoneNumberLoginView>();
  ptr->client_manager = client_manager;
  ptr->toast_overlay = toast_overlay;
  ptr->setup();
  return ptr;
}
} // namespace Views
} // namespace FlyingPaper
