#include "peel/Adw/Application.h"
#include "ApplicationWindow.hpp"
#include "Telegram/ClientManager.hpp"
#include "Telegram/Session.hpp"
#include "gtk/gtk.h"
#include "peel/Gdk/Display.h"
#include "peel/Gio/ActionMap.h"
#include "peel/Gio/ApplicationFlags.h"
#include "peel/Gio/File.h"
#include "peel/Gio/SimpleAction.h"
#include "peel/Gtk/CssProvider.h"
#include "peel/Gtk/StyleContext.h"
#include "peel/Gtk/Window.h"
#include <Application.hpp>
#include <Config.hpp>
#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>

namespace FlyingPaper {
namespace Application {
PEEL_CLASS_IMPL(Application, "FlyingPaperApplication", Adw::Application);
RefPtr<Application> Application::create() {
  return Object::create<Application>(
      prop_application_id(), "org.jumptoskyfree.flyingpaper", prop_flags(),
      Gio::Application::Flags::DEFAULT_FLAGS);
}
void Application::Class::init() {
  override_vfunc_activate<Application>();
  override_vfunc_dispose<Application>();
  override_vfunc_startup<Application>();
}
inline void Application::vfunc_startup() {
  parent_vfunc_startup<Application>();
  RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
  css_provider->load_from_resource(
      "/org/jumptoskyfree/flyingpaper/styles/style.css");

  auto display = Gdk::Display::get_default();
  Gtk::StyleContext::add_provider_for_display(display, css_provider,
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
}
inline void Application::vfunc_dispose() {
  parent_vfunc_dispose<Application>();
  client_manager->stop_loop();
}
void Application::vfunc_activate() {
  parent_vfunc_activate<Application>();

  auto session = Session::Session::get();
  Telegram::ClientManager::ClientManagerAuthorizationParams params = {
      .api_id = FlyingPaper::Config::app_id,
      .api_hash = FlyingPaper::Config::app_hash,
      .system_language = "en",
      .application_version = "0.1.0"};

  session->set_client(Telegram::ClientManager::create(params));
  client_manager = session->get_client();
  auto *window = ApplicationWindow::ApplicationWindow::create(this);
  client_manager->authorize();
  Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateAuthorizationState::ID,
      [window](const Telegram::ClientManager::SharedObject &) {
        window->setup();
        window->present();
      });
  client_manager->start_loop();
}
inline void Application::init(Class *) {
  RefPtr<Gio::SimpleAction> action = Gio::SimpleAction::create("quit", nullptr);
  action->connect_activate(this, &Application::action_quit);
  cast<Gio::ActionMap>()->add_action(action);
  set_accels_for_action("app.quit", (const char *[]){"<Ctrl>Q", nullptr});
}
void Application::action_quit(Gio::SimpleAction *, GLib::Variant *) {
  client_manager->stop_loop();
  quit();
}
void Application::action_about(Gio::SimpleAction *, GLib::Variant *) {
  Gtk::Window *parent_window = get_active_window();
  if (parent_window and !parent_window->check_type<Gtk::Window>())
    parent_window = nullptr;

  // TODO: show informations about the application.
}
} // namespace Application
} // namespace FlyingPaper
