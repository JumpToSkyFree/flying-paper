#ifndef FLYING_PAPER_APPLICATION_WINDOW_HPP
#define FLYING_PAPER_APPLICATION_WINDOW_HPP

#include "Telegram/ClientManager.hpp"
#include "peel/Adw/NavigationSplitView.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Adw/ViewSwitcher.h"
#include "peel/Gtk/Application.h"
#include <peel/Adw/ApplicationWindow.h>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

using namespace peel;

namespace FlyingPaper {
namespace ApplicationWindow {
class ApplicationWindow final : public Adw::ApplicationWindow {
  PEEL_SIMPLE_CLASS(ApplicationWindow, Adw::ApplicationWindow);
  inline void init(Class *);

  Adw::OverlaySplitView *overlay_split_view;
  Adw::NavigationSplitView *navigation_split_view;
  void set_authenticated_content();

  Adw::ViewStack *view_stack_unauthenticated_content;
  void set_unauthenticated_content();

  std::shared_ptr<Telegram::ClientManager> client_manager;

public:
  static ApplicationWindow *create(Gtk::Application *app);
};
} // namespace ApplicationWindow
} // namespace FlyingPaper
#endif
