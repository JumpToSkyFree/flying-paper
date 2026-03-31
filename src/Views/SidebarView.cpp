#include "Avatar.hpp"
#include "Telegram/ClientManager.hpp"
#include "glib.h"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/Toast.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Gdk/Texture.h"
#include "peel/Gio/File.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/HeaderBar.h"
#include "peel/Gtk/LayoutManager.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Picture.h"
#include "peel/Gtk/Widget.h"
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <Views/SidebarView.hpp>
#include <cstdint>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/signal.h>
#include <td/telegram/td_api.h>

namespace FlyingPaper::Views {
PEEL_CLASS_IMPL(Sidebar, "FlyingPaperSidebar", Gtk::Widget);
inline void Sidebar::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<Sidebar>();
}
inline void Sidebar::vfunc_dispose() {
  parent_vfunc_dispose<Sidebar>();
  if (container) {
    container->unparent();
    container = nullptr;
  }
}
void Sidebar::make_profile_avatar() {
  avatar = Widgets::Avatar::create_from_text(true, "My Name", 32);
  avatar->connect_signal("avatar-clicked",
                         [](Widgets::Avatar *) { g_print("Avatar clicked."); });
}
FloatPtr<Adw::HeaderBar> Sidebar::make_header_bar() {
  FloatPtr<Adw::HeaderBar> header_bar = Adw::HeaderBar::create();
  make_profile_avatar();
  header_bar->add_css_class("flat");
  header_bar->pack_start(avatar);
  return header_bar;
}
inline void Sidebar::init(Class *) {
  FloatPtr<Gtk::Box> container =
      Gtk::Box::create(Gtk::Orientation::VERTICAL, 16);
  FloatPtr<Adw::HeaderBar> header_bar = make_header_bar();

  toast_overlay = Adw::ToastOverlay::create();
  toast_overlay->set_child(container);

  container->append(std::move(header_bar));

  set_parent(toast_overlay);
}
FloatPtr<Sidebar> Sidebar::create() {
  FloatPtr<Sidebar> ptr = Object::create<Sidebar>();
  ptr->setup();
  return ptr;
}
void Sidebar::set_avatar_fullname(const td::td_api::user &user) {
  std::string fullname;
  if (user.first_name_.length()) {
    fullname = user.first_name_;
  } else if (user.last_name_.length()) {
    fullname += " " + user.last_name_;
  }
  avatar->set_text(fullname.c_str());
}
void Sidebar::set_avatar_image(const td::td_api::file &file) {
  RefPtr<Gio::File> image_file =
      Gio::File::create_for_path(file.local_->path_.c_str());
  RefPtr<Gtk::Picture> picture = Gtk::Picture::create_for_file(image_file);
  avatar->set_picture(picture);
}
void Sidebar::handle_user_profile_image_download_request(
    const td::td_api::file &file, std::uint64_t file_update_subscription_id) {
  if (file.local_->is_downloading_completed_) {
    set_avatar_image(file);
    Telegram::ClientManagerAccessor::unsubscribe(td::td_api::updateFile::ID,
                                                 file_update_subscription_id);
  }
}
void Sidebar::handle_user_profile_image(
    const td::td_api::object_ptr<td::td_api::profilePhoto> &profile_photo) {
  std::int32_t image_id = profile_photo->small_->id_;
  if (profile_photo->small_->local_ and
      profile_photo->small_->local_->is_downloading_completed_) {
    set_avatar_image(*profile_photo->small_);
  } else if (profile_photo->small_->remote_ and
             profile_photo->small_->remote_->id_ != "0") {
    Telegram::ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::downloadFile>(
            profile_photo->small_->id_, 1, 0, 0, false),
        [this](const Telegram::ClientManager::SharedObject &obj) {
          if (obj->get_id() == td::td_api::error::ID) {
            const auto &error =
                Telegram::ClientManager::cast<td::td_api::error>(obj);
            RefPtr<Adw::Toast> toast =
                Adw::Toast::create(error.message_.c_str());
            toast_overlay->add_toast(toast);
          }
        });
    file_update_subscription_id = Telegram::ClientManagerAccessor::subscribe(
        td::td_api::updateFile::ID,
        [this, image_id](const Telegram::ClientManager::SharedObject &obj) {
          if (obj->get_id() == td::td_api::updateFile::ID) {
            const auto &image_file =
                Telegram::ClientManager::cast<td::td_api::updateFile>(obj);
            if (image_file.file_->id_ == image_id) {
              handle_user_profile_image_download_request(
                  *image_file.file_, file_update_subscription_id);
            }
          }
        });
  }
}
void Sidebar::handle_get_user_request(
    const Telegram::ClientManager::SharedObject &obj) {
  switch (obj->get_id()) {
  case td::td_api::user::ID: {
    const auto &user = Telegram::ClientManager::cast<td::td_api::user>(obj);
    set_avatar_fullname(user);
    if (user.profile_photo_)
      handle_user_profile_image(user.profile_photo_);
    break;
  }
  case td::td_api::error::ID: {
    const auto &error = Telegram::ClientManager::cast<td::td_api::error>(obj);
    RefPtr<Adw::Toast> toast = Adw::Toast::create(error.message_.c_str());
    toast_overlay->add_toast(toast);
    break;
  }
  }
}
void Sidebar::setup() {
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::getMe>(),
      [this](const Telegram::ClientManager::SharedObject &obj) {
        handle_get_user_request(obj);
      });
}
} // namespace FlyingPaper::Views
