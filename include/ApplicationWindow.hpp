#ifndef FLYING_PAPER_APPLICATION_WINDOW_HPP
#define FLYING_PAPER_APPLICATION_WINDOW_HPP

#include "Telegram/ClientManager.hpp"
#include "peel/Adw/NavigationSplitView.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Adw/ViewSwitcher.h"
#include "peel/Gtk/Application.h"
#include <cstdint>
#include <peel/Adw/ApplicationWindow.h>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

using namespace peel;

namespace FlyingPaper {
namespace ApplicationWindow {
class ApplicationWindow final : public Adw::ApplicationWindow {
  PEEL_SIMPLE_CLASS(ApplicationWindow, Adw::ApplicationWindow)
  inline void init(Class *);

  RefPtr<Adw::OverlaySplitView> overlay_split_view;
  RefPtr<Adw::NavigationSplitView> navigation_split_view;
  RefPtr<Adw::ToastOverlay> toast_overlay;
  void set_authenticated_content();
  void handle_authentication(std::int32_t object_id);

  RefPtr<Adw::ViewStack> view_stack_unauthenticated_content;
  void set_unauthenticated_content();

  std::shared_ptr<Telegram::ClientManager> client_manager;

public:
  static ApplicationWindow *create(Gtk::Application *app);
};
} // namespace ApplicationWindow
} // namespace FlyingPaper
#endif
