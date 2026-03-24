#include "Telegram/ClientManager.hpp"
#include "Views/PhoneNumberLoginView.hpp"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Adw/ToolbarView.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Adw/ViewSwitcherBar.h"
#include <ApplicationWindow.hpp>
#include <Config/Config.hpp>
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <peel/Adw/Adw.h>
#include <peel/FloatPtr.h>
#include <peel/GLib/GLib.h>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>

namespace FlyingPaper {
namespace ApplicationWindow {
PEEL_CLASS_IMPL(FlyingPaper::ApplicationWindow::ApplicationWindow,
                "FlyingPaperApplicationWindow", Adw::ApplicationWindow);
inline void ApplicationWindow::Class::init() { return; }
inline void ApplicationWindow::init(Class *) {
  set_title("Flying Paper");
  set_default_size(700, 650);

  Telegram::ClientManager::ClientManagerAuthorizationParams params = {
      .api_id = FlyingPaper::Config::app_id,
      .api_hash = FlyingPaper::Config::app_hash,
      .system_language = "en",
      .application_version = "0.1.0"};

  this->client_manager = Telegram::ClientManager::create(params);
  this->client_manager->start_loop();
  this->client_manager->authorize();

  this->client_manager->on_authentication([this](std::int32_t object_id) {
    this->handle_authentication(object_id);
  });
}
void ApplicationWindow::handle_authentication(std::int32_t object_id) {
  switch (object_id) {
  case td::td_api::authorizationStateWaitPhoneNumber::ID: {
    set_unauthenticated_content();
    break;
  }
  case td::td_api::authorizationStateReady::ID: {
    set_authenticated_content();
    break;
  }
  }
}
void ApplicationWindow::set_authenticated_content() {
  if (toast_overlay->get_parent())
    toast_overlay->unparent();
  overlay_split_view = Adw::OverlaySplitView::create();
  overlay_split_view->set_min_sidebar_width(300);
  overlay_split_view->set_max_sidebar_width(350);
  overlay_split_view->set_pin_sidebar(false);
  overlay_split_view->set_collapsed(true);
  overlay_split_view->set_show_sidebar(true);

  navigation_split_view = Adw::NavigationSplitView::create();
  navigation_split_view->set_min_sidebar_width(300);
  navigation_split_view->set_max_sidebar_width(400);
  overlay_split_view->set_content(this->navigation_split_view);
  set_content(overlay_split_view);
}
void ApplicationWindow::set_unauthenticated_content() {
  if (toast_overlay->get_parent())
    toast_overlay->unparent();
  view_stack_unauthenticated_content = Adw::ViewStack::create();

  FloatPtr<Adw::HeaderBar> header_bar = Adw::HeaderBar::create();
  FloatPtr<Adw::ViewSwitcherBar> view_switcher_bar =
      Adw::ViewSwitcherBar::create();
  toast_overlay = Adw::ToastOverlay::create();

  RefPtr<Views::PhoneNumberLoginView> phone_number_view =
      Views::PhoneNumberLoginView::create(this->client_manager, toast_overlay);
  view_stack_unauthenticated_content->add_titled_with_icon(
      phone_number_view, "phone_number", "Phone Number",
      "input-dialpad-symbolic");

  // TODO: Add login with QR code.

  view_switcher_bar->set_stack(view_stack_unauthenticated_content);
  view_switcher_bar->set_reveal(true);
  toast_overlay->set_child(view_stack_unauthenticated_content);

  FloatPtr<Adw::ToolbarView> toolbar_view = Adw::ToolbarView::create();
  toolbar_view->set_content(toast_overlay);
  toolbar_view->add_top_bar(std::move(header_bar));
  toolbar_view->add_bottom_bar(std::move(view_switcher_bar));
  set_content(std::move(toolbar_view));
}
ApplicationWindow *ApplicationWindow::create(Gtk::Application *app) {
  ApplicationWindow *window =
      Object::create<ApplicationWindow>(prop_application(), app);
  return window;
}
} // namespace ApplicationWindow
} // namespace FlyingPaper
