#ifndef FLYING_PAPER_SIDEBAR_VIEW_HPP
#define FLYING_PAPER_SIDEBAR_VIEW_HPP

#include "Avatar.hpp"
#include "Telegram/ClientManager.hpp"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/GestureClick.h"
#include "peel/Gtk/MenuButton.h"
#include "peel/Gtk/Widget.h"
#include <peel/FloatPtr.h>
#include <peel/Gtk/Box.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>

using namespace peel;

namespace FlyingPaper::Views {
class Sidebar final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(Sidebar, Gtk::Widget)
  friend Gtk::Widget;

  inline void init(Class *);
  inline void vfunc_dispose();

  Gtk::Box *container;
  RefPtr<Adw::ToastOverlay> toast_overlay;
  RefPtr<Gtk::GestureClick> avatar_click_controller;
  RefPtr<Widgets::Avatar> avatar;
  std::uint64_t file_update_subscription_id;

  FloatPtr<Adw::HeaderBar> make_header_bar();
  FloatPtr<Gtk::MenuButton> make_menu();
  void make_profile_avatar();

  void set_avatar_fullname(const td::td_api::user &);
  void set_avatar_image(const td::td_api::file &);
  void handle_get_user_request(const Telegram::ClientManager::SharedObject &);
  void handle_user_profile_image(
      const td::td_api::object_ptr<td::td_api::profilePhoto> &);
  void handle_user_profile_image_download_request(
      const td::td_api::file &file, std::uint64_t file_update_subscription_id);

public:
  static FloatPtr<Sidebar> create();
};
} // namespace FlyingPaper::Views

#endif
