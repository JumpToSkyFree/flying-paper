#include "peel/Adw/Application.h"
#include "ApplicationWindow.hpp"
#include "peel/Gio/ActionMap.h"
#include "peel/Gio/ApplicationFlags.h"
#include "peel/Gio/SimpleAction.h"
#include "peel/Gtk/Window.h"
#include <Application.hpp>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

namespace FlyingPaper {
namespace Application {
PEEL_CLASS_IMPL(Application, "FlyingPaperApplication", Adw::Application);
RefPtr<Application> Application::create() {
  return Object::create<Application>(
      prop_application_id(), "org.jumptoskyfree.flyingpaper", prop_flags(),
      Gio::Application::Flags::DEFAULT_FLAGS);
}
void Application::Class::init() { override_vfunc_activate<Application>(); }
void Application::vfunc_activate() {
  parent_vfunc_activate<Application>();

  // TODO: Create the main application window.
  auto *window = ApplicationWindow::ApplicationWindow::create(this);
  window->present();
}
inline void Application::init(Class *) {
  RefPtr<Gio::SimpleAction> action = Gio::SimpleAction::create("quit", nullptr);
  action->connect_activate(this, &Application::action_quit);
  cast<Gio::ActionMap>()->add_action(action);
  set_accels_for_action("app.quit", (const char *[]){"<Ctrl>Q", nullptr});
}
void Application::action_quit(Gio::SimpleAction *, GLib::Variant *) { quit(); }
void Application::action_about(Gio::SimpleAction *, GLib::Variant *) {
  Gtk::Window *parent_window = get_active_window();
  if (parent_window && !parent_window->check_type<Gtk::Window>())
    parent_window = nullptr;

  // TODO: Show informations about the application.
}
} // namespace Application
} // namespace FlyingPaper
