#include "Telegram/ClientManager.hpp"
#include "Views/VerificationCodeInsertView.hpp"
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

namespace FlyingPaper::Views {
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

  status_page = Adw::StatusPage::create();

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

  set_parent(status_page);
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
  if (wait_code_subscribtion_id != 0) {
    Telegram::ClientManagerAccessor::unsubscribe(
        td::td_api::updateAuthorizationState::ID, wait_code_subscribtion_id);
    wait_code_subscribtion_id = 0;
  }
  parent_vfunc_dispose<PhoneNumberLoginView>();
}
void PhoneNumberLoginView::handle_send_code(VerificationCodeInsertView *) {
  std::string verification_code = verification_view->get_verification_code();
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::checkAuthenticationCode>(
          verification_code),
      [this](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::ok>(
            obj, nullptr,
            [this](const std::shared_ptr<td::td_api::error> &error) {
              if (error->message_ == "PHONE_CODE_INVALID") {
                toast = Adw::Toast::create("Phone code invalid");
                this->toast_overlay->dismiss_all();
                this->toast_overlay->add_toast(toast);
              }
            });
      });
}
inline void PhoneNumberLoginView::init(Class *) {
  FloatPtr<Adw::ViewStack> _view_stack = Adw::ViewStack::create();
  view_stack = _view_stack;
  view_stack->set_transition_duration(true);
  view_stack->set_transition_duration(500);
  set_halign(Gtk::Align::CENTER);
  set_valign(Gtk::Align::CENTER);
  insert_view = Object::create<PhoneNumberInsertView>();
  verification_view = VerificationCodeInsertView::create();
  verification_view->set_n_entries(5);
  verification_view->connect_signal(
      "continue-clicked",
      [this](VerificationCodeInsertView *verification_view) {
        handle_send_code(verification_view);
      });
  verification_view->set_view_stack(view_stack);
  view_stack->add_named(insert_view, "phone-number-insert");
  view_stack->add_named(verification_view, "verification");
  insert_view->connect_signal("next-clicked", [this](PhoneNumberInsertView *) {
    Telegram::ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::setAuthenticationPhoneNumber>(
            insert_view->get_phone_number(), nullptr),
        [this](const Telegram::ClientManager::SharedObject &obj) {
          Telegram::ClientManagerAccessor::handle<td::td_api::ok>(
              obj, nullptr, [this](std::shared_ptr<td::td_api::error> error) {
                if (error->message_ == "PHONE_NUMBER_INVALID") {
                  toast = Adw::Toast::create("Invalid phone number");
                  this->toast_overlay->dismiss_all();
                  this->toast_overlay->add_toast(toast);
                }
              });
        });
  });
  setup();
  set_parent(view_stack);
}
void PhoneNumberLoginView::setup() {
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::getAuthorizationState>(),
      [this](const Telegram::ClientManager::SharedObject &obj) {
        switch (obj->get_id()) {
        case td::td_api::authorizationStateWaitPhoneNumber::ID:
          this->view_stack->set_visible_child_name("phone-number-insert");
          break;
        case td::td_api::authorizationStateWaitCode::ID:
          this->view_stack->set_visible_child_name("verification");
          break;
        }
      });
  wait_code_subscribtion_id = Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateAuthorizationState::ID,
      [this](const Telegram::ClientManager::SharedObject &obj) {
        const auto &object =
            Telegram::ClientManager::cast<td::td_api::updateAuthorizationState>(
                obj);
        if (object.authorization_state_->get_id() ==
            td::td_api::authorizationStateWaitCode::ID) {
          view_stack->set_visible_child(verification_view);
        }
      });
}
FloatPtr<PhoneNumberLoginView> PhoneNumberLoginView::create(
    peel::RefPtr<peel::Adw::ToastOverlay> toast_overlay) {
  auto ptr = Object::create<PhoneNumberLoginView>();
  ptr->toast_overlay = toast_overlay;
  return ptr;
}
} // namespace FlyingPaper::Views
