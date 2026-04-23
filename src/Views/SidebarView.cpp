#include "Telegram/ClientManager.hpp"
#include "Telegram/Session.hpp"
#include "Widgets/Avatar.hpp"
#include "Widgets/ChatListItem.hpp"
#include "Widgets/ScrolledContainer.hpp"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/OverlaySplitView.h"
#include "peel/Adw/Toast.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/Gdk/Texture.h"
#include "peel/Gio/File.h"
#include "peel/Gio/Menu.h"
#include "peel/Gtk/Application.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/HeaderBar.h"
#include "peel/Gtk/LayoutManager.h"
#include "peel/Gtk/MenuButton.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Picture.h"
#include "peel/Gtk/Popover.h"
#include "peel/Gtk/PopoverMenu.h"
#include "peel/Gtk/ScrolledWindow.h"
#include "peel/Gtk/Widget.h"
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <Views/SidebarView.hpp>
#include <memory>
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
  if (update_new_chat_subscription != 0) {
    Telegram::ClientManagerAccessor::unsubscribe(
        td::td_api::updateNewMessage::ID, update_new_chat_subscription);
  }
  if (update_chat_position != 0) {
    Telegram::ClientManagerAccessor::unsubscribe(
        td::td_api::updateChatPosition::ID, update_chat_position);
  }
}
void Sidebar::make_profile_avatar() {
  avatar = Widgets::Avatar::create_from_text(true, "My Name", 32);
  avatar->connect_signal("avatar-clicked", [](Widgets::Avatar *) {
    // TODO: access overlay_split_view and open the sidebar.
  });
}
FloatPtr<Gtk::MenuButton> Sidebar::make_menu() {
  RefPtr<Gio::Menu> menu = Gio::Menu::create();
  menu->append("Quit", "app.quit");

  FloatPtr<Gtk::PopoverMenu> popover_menu =
      Gtk::PopoverMenu::create_from_model(menu);

  FloatPtr<Gtk::MenuButton> ret = Gtk::MenuButton::create();
  ret->set_popover(std::move(popover_menu));
  ret->set_icon_name("open-menu-symbolic");
  ret->set_tooltip_text("Menu");
  ret->set_focus_on_click(true);
  return ret;
}
FloatPtr<Adw::HeaderBar> Sidebar::make_header_bar() {
  FloatPtr<Adw::HeaderBar> header_bar = Adw::HeaderBar::create();
  make_profile_avatar();
  auto session = Session::Session::get();
  session->request_context<td::td_api::user>(
      "me", [this](std::shared_ptr<void> data) {
        const auto &user = Session::Session::cast<td::td_api::user>(data);
        Telegram::ClientManagerAccessor::handle<td::td_api::user>(
            user,
            [this](const std::shared_ptr<td::td_api::user> &user) {
              set_avatar_fullname(*user);
              if (user->profile_photo_) {
                auto session = Session::Session::get();
                auto client = session->get_client();
                client->download_file(
                    user->profile_photo_->small_, 1,
                    [this](const td::td_api::file &file) {
                      set_avatar_image(file);
                    },
                    [this](const td::td_api::error &error) {
                      RefPtr<Adw::Toast> toast =
                          Adw::Toast::create(error.message_.c_str());
                      toast_overlay->add_toast(toast);
                    });
              }
            },
            [this](const std::shared_ptr<td::td_api::error> &error) {
              RefPtr<Adw::Toast> toast =
                  Adw::Toast::create(error->message_.c_str());
              toast_overlay->add_toast(toast);
            });
      });
  header_bar->add_css_class("flat");
  header_bar->pack_start(avatar);
  header_bar->pack_end(make_menu());
  return header_bar;
}
inline void Sidebar::init(Class *) {
  RefPtr<Gtk::Box> container = Gtk::Box::create(Gtk::Orientation::VERTICAL, 0);
  FloatPtr<Adw::HeaderBar> header_bar = make_header_bar();

  RefPtr<Widgets::ScrolledContainer> scrolled_container =
      Widgets::ScrolledContainer::create();

  // TODO: Make recursive calls to loadChats until we receive an 404 error,
  // indicating that no more chats are available.
  scrolled_container->on_threshold_reached([]() {
    Telegram::ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::loadChats>(),
        [](const Telegram::ClientManager::SharedObject &obj) {
          Telegram::ClientManagerAccessor::handle<td::td_api::ok>(
              obj, nullptr, [](const std::shared_ptr<td::td_api::error> &) {
                // TODO: No more loading, done.
              });
        });
  });

  RefPtr<Gtk::ScrolledWindow> scrolled_win =
      scrolled_container->get_scrolled_window();

  RefPtr<Gtk::Box> chats_box = Gtk::Box::create(Gtk::Orientation::VERTICAL, 8);
  scrolled_win->set_child(chats_box);
  scrolled_win->set_vexpand(true);
  chats_box->set_vexpand(true);

  toast_overlay = Adw::ToastOverlay::create();
  toast_overlay->set_child(container);
  auto session = Session::Session::get();
  session->set_context<RefPtr<Adw::ToastOverlay>>("sidebar-toast-overlay",
                                                  toast_overlay);

  container->append(std::move(header_bar));
  container->append(scrolled_container);
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::getChats>(std::move(chat_list), 300),
      [this, _container = container,
       chats_box](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::chats>(
            obj,
            [chats_box,
             _container](const std::shared_ptr<td::td_api::chats> &chats) {
              for (std::size_t i = 0; i < chats->chat_ids_.size(); i++) {
                Telegram::ClientManagerAccessor::send(
                    td::td_api::make_object<td::td_api::getChat>(
                        chats->chat_ids_[i]),
                    [chats_box](
                        const Telegram::ClientManager::SharedObject &obj) {
                      Telegram::ClientManagerAccessor::handle<td::td_api::chat>(
                          obj,
                          [chats_box](
                              const std::shared_ptr<td::td_api::chat> &chat) {
                            FloatPtr<Widgets::ChatListItem> chat_list_item =
                                Widgets::ChatListItem::create();
                            chat_list_item->set_chat(chat);
                            chats_box->append(std::move(chat_list_item));
                          },
                          nullptr);
                    });
              }
            },
            [this](const std::shared_ptr<td::td_api::error> &error) {
              RefPtr<Adw::Toast> toast =
                  Adw::Toast::create(error->message_.c_str());
              toast_overlay->add_toast(std::move(toast));
            });
      });

  update_new_chat_subscription = Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateNewMessage::ID,
      [chats_box](const Telegram::ClientManager::SharedObject &obj) {
        const auto &new_message =
            Telegram::ClientManager::cast_ptr<td::td_api::updateNewMessage>(
                obj);
        Gtk::Widget *it = chats_box->get_first_child();
        while (it) {
          auto chat_list_item = static_cast<Widgets::ChatListItem *>(it);
          if (chat_list_item->get_chat_id() ==
              new_message->message_->chat_id_) {
            chat_list_item->set_new_message(new_message);
            chats_box->reorder_child_after(it, nullptr);
            break;
          }
          it = it->get_next_sibling();
        }
      });

  update_chat_position = Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateChatPosition::ID,
      [chats_box](const Telegram::ClientManager::SharedObject &obj) {
        const auto &_update_chat_position =
            Telegram::ClientManager::cast_ptr<td::td_api::updateChatPosition>(
                obj);
        Gtk::Widget *it = chats_box->get_first_child();
        std::int64_t pos{0};
        while (it) {
          if (pos == _update_chat_position->position_->order_) {
            chats_box->reorder_child_after(it, chats_box->get_next_sibling());
            break;
          }
          pos++;
          it = it->get_next_sibling();
        }
      });

  set_parent(toast_overlay);
}
FloatPtr<Sidebar> Sidebar::create() { return Object::create<Sidebar>(); }
void Sidebar::set_avatar_fullname(const td::td_api::user &user) {
  std::string fullname;
  if (user.first_name_.length()) {
    fullname = user.first_name_;
  }
  if (user.last_name_.length()) {
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
} // namespace FlyingPaper::Views
