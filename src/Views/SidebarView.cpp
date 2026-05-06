#include "Telegram/ClientManager.hpp"
#include "Telegram/Session.hpp"
#include "Widgets/Avatar.hpp"
#include "Widgets/ChatListItem.hpp"
#include "Widgets/LastMessage.hpp"
#include "Widgets/ScrolledContainer.hpp"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/NavigationPage.h"
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
#include "peel/Gtk/Label.h"
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
#include <cassert>
#include <cstdint>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/signal.h>
#include <string>
#include <td/telegram/td_api.h>
#include <unordered_map>

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
  // FIX: Make sure to remove sidebar-toast-overlay from session context.
  if (update_new_chat_subscription) {
    Telegram::ClientManagerAccessor::unsubscribe(
        td::td_api::updateNewMessage::ID, update_new_chat_subscription);
  }
  if (update_chat_position) {
    Telegram::ClientManagerAccessor::unsubscribe(
        td::td_api::updateChatPosition::ID, update_chat_position);
  }
  if (update_new_chat) {
    Telegram::ClientManagerAccessor::unsubscribe(
        td::td_api::updateChatPosition::ID, update_new_chat);
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

  auto session = Session::Session::get();
  session->set_context("chats",
                       std::make_shared<std::unordered_map<
                           std::int64_t, std::shared_ptr<td::td_api::chat>>>());

  RefPtr<Widgets::ScrolledContainer> scrolled_container =
      Widgets::ScrolledContainer::create();
  RefPtr<Gtk::ScrolledWindow> scrolled_win =
      scrolled_container->get_scrolled_window();

  chats_box = Gtk::Box::create(Gtk::Orientation::VERTICAL, 0);
  scrolled_win->set_child(chats_box);
  scrolled_win->set_vexpand(true);
  chats_box->set_vexpand(true);

  toast_overlay = Adw::ToastOverlay::create();
  toast_overlay->set_child(container);

  session->set_context<RefPtr<Adw::ToastOverlay>>("sidebar-toast-overlay",
                                                  toast_overlay);

  container->append(std::move(header_bar));
  container->append(scrolled_container);

  update_new_chat = Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateNewChat::ID,
      [this](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor ::handle<td::td_api::updateNewChat>(
            obj,
            [this](const std::shared_ptr<td::td_api::updateNewChat> &new_chat) {
              const auto &chat =
                  Telegram::ClientManager::cast_ptr<td::td_api::chat,
                                                    td::td_api::chat>(
                      std::move(new_chat->chat_));
              FloatPtr<Widgets::ChatListItem> chat_list_item =
                  Widgets::ChatListItem::create();
              chat_list_item->set_chat(chat);
              chat_list_item->connect_signal(
                  "chat-list-item-clicked",
                  [this](Widgets::ChatListItem *item) {
                    if (active_item && active_item != item) {
                      auto button = active_item->get_button();
                      button->set_active(false);
                    }
                    if (cb) {
                      cb(item);
                    }
                    active_item = item;
                  });
              chats_box->append(std::move(chat_list_item));
            },
            nullptr);
      });

  update_new_chat_subscription = Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateNewMessage::ID,
      [this](const Telegram::ClientManager::SharedObject &obj) {
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
      [this](const Telegram::ClientManager::SharedObject &obj) {
        const auto &_update_chat_position =
            Telegram::ClientManager::cast_ptr<td::td_api::updateChatPosition>(
                obj);
        Widgets::ChatListItem *widget =
            find_chat_by_id(_update_chat_position->chat_id_);
        if (widget) {
          widget->set_order(_update_chat_position->position_->order_);
          if (_update_chat_position->position_->order_ == 0) {
            chats_box->reorder_child_after(widget, nullptr);
            return;
          }
          order_chat_item_position(widget);
        }
      });

  Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateChatLastMessage::ID,
      [this](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<
            td::td_api::updateChatLastMessage>(
            obj,
            [this](const std::shared_ptr<td::td_api::updateChatLastMessage>
                       &last_message) {
              const auto &last_message_ =
                  Telegram::ClientManager::cast_ptr<td::td_api::message,
                                                    td::td_api::message>(
                      std::move(last_message->last_message_));
              Widgets::ChatListItem *item =
                  find_chat_by_id(last_message->chat_id_);
              if (item) {
                auto button = item->get_button();
                item->set_last_message(last_message_);
                RefPtr<Widgets::LastMessage> last_message =
                    item->get_last_message();
                if (last_message_) {
                  Telegram::ClientManagerAccessor::send(
                      td::td_api::make_object<td::td_api::getChat>(
                          last_message_->chat_id_),
                      [last_message, last_message_](
                          const Telegram::ClientManager::SharedObject &obj) {
                        Telegram::ClientManagerAccessor::handle<
                            td::td_api::chat>(
                            obj,
                            [last_message, last_message_](
                                const std::shared_ptr<td::td_api::chat> &chat) {
                              // TODO: Add two checkmarks to indicate that the
                              // outgoing message was by recipient.
                              const bool isRead =
                                  last_message_->is_outgoing_ ||
                                  last_message_->id_ <=
                                      chat->last_read_inbox_message_id_;
                              last_message->set_read(isRead);
                            },
                            nullptr);
                      });
                }
              }
              if (item) {
                for (const auto &pos : last_message->positions_) {
                  if (pos->list_->get_id() == td::td_api::chatListMain::ID) {
                    item->set_order(pos->order_);
                  }
                }
                order_chat_item_position(item);
              }
            },
            [](const std::shared_ptr<td::td_api::error> &) {
              // TODO: Show error if last message of every contact can't be
              // synced.
            });
      });

  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::loadChats>(nullptr, 30),
      [](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::ok>(obj, nullptr,
                                                                nullptr);
      });

  scrolled_container->on_threshold_reached([]() {
    Telegram::ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::loadChats>(
            td::td_api::make_object<td::td_api::chatListMain>(), 30),
        [](const Telegram::ClientManager::SharedObject &obj) {
          Telegram::ClientManagerAccessor::handle<td::td_api::ok>(obj, nullptr,
                                                                  nullptr);
        });
  });

  set_parent(toast_overlay);
}
void Sidebar::order_chat_item_position(Widgets::ChatListItem *item) {
  Gtk::Widget *it = chats_box->get_first_child();
  Gtk::Widget *prev = nullptr;

  while (it) {
    auto current_chat = static_cast<Widgets::ChatListItem *>(it);

    if (current_chat == item) {
      it = it->get_next_sibling();
      continue;
    }

    if (item->get_order() > current_chat->get_order() ||
        (item->get_order() == current_chat->get_order() &&
         item->get_chat_id() > current_chat->get_chat_id())) {
      if (prev == nullptr) {
        chats_box->reorder_child_after(item, nullptr);
      } else {
        chats_box->reorder_child_after(item, prev);
      }
      return;
    }

    prev = it;
    it = it->get_next_sibling();
  }

  // If it's the smallest order found, move it to the very bottom
  chats_box->reorder_child_after(item, prev);
}

Widgets::ChatListItem *Sidebar::find_chat_by_id(std::int64_t id) {
  Gtk::Widget *it = chats_box->get_first_child();
  while (it) {
    auto chat_list_item = static_cast<Widgets::ChatListItem *>(it);
    if (chat_list_item->get_chat_id() == id) {
      return chat_list_item;
    }
    it = it->get_next_sibling();
  }
  return nullptr;
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
void Sidebar::set_on_chat_item_selected(Callback cb) {
  this->cb = std::move(cb);
}
} // namespace FlyingPaper::Views
