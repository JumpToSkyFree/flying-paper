#ifndef FLYING_PAPER_APPLICATION_WINDOW_HPP
#define FLYING_PAPER_APPLICATION_WINDOW_HPP

#include "Views/ChatView.hpp"
#include "peel/Adw/NavigationPage.h"
#include "peel/Adw/NavigationSplitView.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Adw/ViewStack.h"
#include "peel/Adw/ViewSwitcher.h"
#include "peel/Gtk/Application.h"
#include <cstdint>
#include <memory>
#include <peel/Adw/ApplicationWindow.h>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <unordered_map>

using namespace peel;

namespace FlyingPaper::ApplicationWindow {
class ApplicationWindow final : public Adw::ApplicationWindow {
  PEEL_SIMPLE_CLASS(ApplicationWindow, Adw::ApplicationWindow)
  inline void init(Class *);

  RefPtr<Adw::OverlaySplitView> overlay_split_view;
  RefPtr<Adw::NavigationSplitView> navigation_split_view;
  RefPtr<Adw::ToastOverlay> toast_overlay;
  peel::SignalConnection active_state_connection;
  void set_authenticated_content();
  void handle_authentication(std::int32_t object_id);
  std::shared_ptr<std::unordered_map<std::int64_t, RefPtr<Views::Chat>>>
      chats_cache;

  RefPtr<Adw::ViewStack> view_stack_unauthenticated_content;
  void set_unauthenticated_content();

  RefPtr<Adw::NavigationPage> sidebar_page;

  inline void init(Class *);
  inline void vfunc_dispose();

public:
  static ApplicationWindow *create(Gtk::Application *app);
  void setup();
};
} // namespace FlyingPaper::ApplicationWindow
#endif
