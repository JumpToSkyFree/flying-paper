#ifndef FLYING_PAPER_PHONE_NUMBER_LOGIN_VIEW_HPP
#define FLYING_PAPER_PHONE_NUMBER_LOGIN_VIEW_HPP
#include "Views/VerificationCodeInsertView.hpp"
#include "peel/Adw/NavigationView.h"
#include "peel/Adw/StatusPage.h"
#include "peel/Adw/Toast.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/Widget.h"
#include <Telegram/ClientManager.hpp>
#include <cstdint>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/Gtk/Gtk.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/signal.h>
using namespace peel;
namespace FlyingPaper::Views {

// TODO: Make a seperate better looking phone number input that have two
// inputs to pick country code and insert phone number, use
// help.getCountriesList to list country code every country.
class PhoneNumberInsertView final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(PhoneNumberInsertView, Gtk::Widget)
  friend Gtk::Widget;

  inline void init(Class *);
  inline void vfunc_dispose();

  RefPtr<Adw::StatusPage> status_page;
  RefPtr<Gtk::Entry> input;
  RefPtr<Gtk::Button> login_button;

  static Signal<PhoneNumberInsertView, void(void)> sig_next_clicked;

  PEEL_SIGNAL_CONNECT_METHOD(next_clicked, sig_next_clicked);

public:
  const char *get_phone_number() const;
};

class PhoneNumberLoginView final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(PhoneNumberLoginView, Gtk::Widget)
  friend Gtk::Widget;
  RefPtr<PhoneNumberInsertView> insert_view;
  RefPtr<VerificationCodeInsertView> verification_view;
  RefPtr<Adw::ViewStack> view_stack;
  std::shared_ptr<Telegram::ClientManager> client_manager;
  RefPtr<Adw::ToastOverlay> toast_overlay;
  RefPtr<Adw::NavigationPage> verification_view_page;
  RefPtr<Adw::Toast> toast;
  std::uint64_t wait_code_subscribtion_id{0};

  inline void init(Class *);
  inline void vfunc_dispose();

  void send_phone_number(const Telegram::ClientManager::SharedObject &obj);
  void handle_authentication_failed(
      const Telegram::ClientManager::SharedObject &obj);
  void handle_send_code(VerificationCodeInsertView *verification_code_view);

public:
  static FloatPtr<PhoneNumberLoginView>
  create(std::shared_ptr<Telegram::ClientManager> client_manager,
         RefPtr<Adw::ToastOverlay> toast_overlay);
  void setup();
};
} // namespace FlyingPaper::Views

#endif
