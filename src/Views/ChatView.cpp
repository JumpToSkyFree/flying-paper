#include "Telegram/ClientManager.hpp"
#include "Widgets/ScrolledContainer.hpp"
#include "glib.h"
#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/NavigationPage.h"
#include "peel/Adw/ToolbarView.h"
#include "peel/GLib/DateTime.h"
#include "peel/GLib/functions.h"
#include "peel/Gtk/Adjustment.h"
#include "peel/Gtk/Align.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/ScrolledWindow.h"
#include "peel/Gtk/Widget.h"
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <Views/ChatView.hpp>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/GObject/Type.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <string>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>

namespace FlyingPaper::Views {
PEEL_CLASS_IMPL(Chat, "FlyingPaperChat", Gtk::Widget)
void Chat::Class::init() {
  set_layout_manager_type(Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<Chat>();
}
inline void Chat::init(Class *) {
  navigation_view = Adw::NavigationView::create();
  toolbar_view = Adw::ToolbarView::create();
  make_header_bar();
  toolbar_view->add_top_bar(header_bar);
  FloatPtr<Adw::NavigationPage> chat_page =
      Adw::NavigationPage::create(toolbar_view, "");
  update_user_status_sub = Telegram::ClientManagerAccessor::subscribe(
      td::td_api::updateUserStatus::ID,
      [this](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::updateUserStatus>(
            obj,
            [this](
                const std::shared_ptr<td::td_api::updateUserStatus> &_status) {
              if (user_id == _status->user_id_) {
                const auto &status =
                    Telegram::ClientManager::cast_ptr<td::td_api::UserStatus,
                                                      td::td_api::UserStatus>(
                        std::move(_status->status_));
                String user_status = handle_user_status(status);
                bottom_label->set_markup(user_status);
              }
            },
            nullptr);
      });
  set_parent(std::move(chat_page));
}
inline void Chat::vfunc_dispose() {
  if (toolbar_view) {
    toolbar_view->unparent();
    toolbar_view = nullptr;
  }
  Telegram::ClientManagerAccessor::unsubscribe(td::td_api::updateUserStatus::ID,
                                               update_user_status_sub);
  parent_vfunc_dispose<Chat>();
}
void Chat::make_header_bar() {
  header_bar = Adw::HeaderBar::create();
  header_bar->set_show_title(false);

  FloatPtr<Gtk::Box> box = Gtk::Box::create(Gtk::Orientation::VERTICAL, 2);
  chat_title = Gtk::Label::create("");
  bottom_label = Gtk::Label::create("");
  box->append(chat_title);
  box->append(bottom_label);
  chat_title->set_halign(Gtk::Align::START);
  bottom_label->set_halign(Gtk::Align::START);
  header_bar->set_show_start_title_buttons(true);
  header_bar->pack_start(std::move(box));
}
void Chat::set_chat_content() {
  chat_messages_container = Gtk::Box::create(Gtk::Orientation::VERTICAL, 16);

  if (this->toolbar_view) {
    scrolled_container = Widgets::ScrolledContainer::create(); // fix typo too
    scrolled_container->on_threshold_reached_start(
        [this]() { fetch_n_messages(std::make_shared<std::int32_t>(20)); });
    auto scrolled_window = scrolled_container->get_scrolled_window();
    scrolled_window->set_child(chat_messages_container);
    toolbar_view->set_content(scrolled_container);

    // FIX: Scroll is at the bottom but top elements are shown.
    std::shared_ptr<std::int32_t> left_messages =
        std::make_shared<std::int32_t>(20);
    fetch_messages();
  }
}
void Chat::fetch_n_messages(std::shared_ptr<std::int32_t> left_messages) {
  if (!(*left_messages))
    return;
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::getChatHistory>(
          this->chat_id, from_message_id, 0, 100, false),
      [this, left_messages](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::messages>(
            obj,
            [this, left_messages](
                const std::shared_ptr<td::td_api::messages> &messages) {
              if (!messages->messages_.empty() && messages->messages_.back()) {
                from_message_id = messages->messages_.back()->id_;
                for (std::size_t i = 0;
                     i < messages->messages_.size() && (*left_messages); i++) {
                  auto message = std::move(messages->messages_[i]);
                  auto _message_content = Telegram::ClientManager::cast_ptr<
                      td::td_api::MessageContent, td::td_api::MessageContent>(
                      std::move(message->content_));
                  Telegram::ClientManagerAccessor::handle<
                      td::td_api::MessageContent, td::td_api::messageText>(
                      _message_content,
                      [this, left_messages](
                          const std::shared_ptr<td::td_api::messageText>
                              &message) {
                        GLib::idle_add([this, message]() {
                          auto label =
                              Gtk::Label::create(message->text_->text_.c_str());
                          label->set_width_chars(128);
                          chat_messages_container->prepend(std::move(label));
                          return G_SOURCE_REMOVE;
                        });
                        *left_messages -= 1;
                      },
                      nullptr);
                }
                fetch_n_messages(left_messages);
              }
            },
            [](const std::shared_ptr<td::td_api::error> &error) {
              // TODO: Show an error if there is a problem with fetching
              // messages.
            });
      });
}
void Chat::fetch_messages() {
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::getChatHistory>(
          this->chat_id, from_message_id, 0, 100, false),
      [this](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::messages>(
            obj,
            [this](const std::shared_ptr<td::td_api::messages> &messages) {
              auto scrolled_window = scrolled_container->get_scrolled_window();
              auto adj = scrolled_window->get_vadjustment();
              double upper = adj->get_upper();
              double page_size = adj->get_page_size();
              if (!messages->messages_.empty() && messages->messages_.back()) {
                from_message_id = messages->messages_.back()->id_;
                for (std::size_t i = 0;
                     i < messages->messages_.size() && !(upper > page_size);
                     i++) {
                  auto message = std::move(messages->messages_[i]);
                  auto _message_content = Telegram::ClientManager::cast_ptr<
                      td::td_api::MessageContent, td::td_api::MessageContent>(
                      std::move(message->content_));
                  Telegram::ClientManagerAccessor::handle<
                      td::td_api::MessageContent, td::td_api::messageText>(
                      _message_content,
                      [this](const std::shared_ptr<td::td_api::messageText>
                                 &message) {
                        GLib::idle_add([this, message]() {
                          auto label =
                              Gtk::Label::create(message->text_->text_.c_str());
                          label->set_width_chars(128);
                          chat_messages_container->prepend(std::move(label));
                          return G_SOURCE_REMOVE;
                        });
                      },
                      nullptr);
                }
                if (!(upper > page_size)) {
                  fetch_messages();
                } else {
                  auto scrolled_window =
                      scrolled_container->get_scrolled_window();
                  auto adj = scrolled_window->get_vadjustment();
                  adj->set_value(adj->get_upper() - adj->get_page_size());
                }
              }
            },
            [](const std::shared_ptr<td::td_api::error> &error) {
              // TODO: Show an error if there is a problem with fetching
              // messages.
            });
      });
}
void Chat::set_header_bar() {
  Telegram::ClientManagerAccessor::send(
      td::td_api::make_object<td::td_api::getChat>(this->chat_id),
      [this](const Telegram::ClientManager::SharedObject &obj) {
        Telegram::ClientManagerAccessor::handle<td::td_api::chat>(
            obj,
            [this](const std::shared_ptr<td::td_api::chat> &chat) {
              std::string title = chat->title_;
              String markup = GLib::strdup_printf("<b>%s</b>", title.c_str());
              chat_title->set_markup(markup);
              const auto &chat_type =
                  Telegram::ClientManager::cast_ptr<td::td_api::ChatType,
                                                    td::td_api::ChatType>(
                      std::move(chat->type_));
              Telegram::ClientManagerAccessor::handle<
                  td::td_api::chatTypePrivate>(
                  chat_type,
                  [this](const std::shared_ptr<td::td_api::chatTypePrivate>
                             &private_chat) {
                    this->user_id = private_chat->user_id_;
                    Telegram::ClientManagerAccessor::send(
                        td::td_api::make_object<td::td_api::getUser>(
                            private_chat->user_id_),
                        [this](
                            const Telegram::ClientManager::SharedObject &obj) {
                          const auto &user = Telegram::ClientManager::cast_ptr<
                              td::td_api::user>(obj);
                          const auto &status =
                              Telegram::ClientManager::cast_ptr<
                                  td::td_api::UserStatus,
                                  td::td_api::UserStatus>(
                                  std::move(user->status_));
                          String time = handle_user_status(status);
                          bottom_label->set_markup(time);
                        });
                  },
                  nullptr);
              Telegram::ClientManagerAccessor::handle<
                  td::td_api::chatTypeBasicGroup>(
                  chat_type,
                  [this](const std::shared_ptr<td::td_api::chatTypeBasicGroup>
                             &basic_group) {
                    // basic_group->ID
                    Telegram::ClientManagerAccessor::send(
                        td::td_api::make_object<
                            td::td_api::getBasicGroupFullInfo>(
                            basic_group->basic_group_id_),
                        [this](
                            const Telegram::ClientManager::SharedObject &obj) {
                          const auto &basic_group_info =
                              Telegram::ClientManager::cast_ptr<
                                  td::td_api::basicGroupFullInfo>(obj);
                          std::int64_t count =
                              basic_group_info->members_.size();
                          String count_str = GLib::strdup_printf(
                              "<span alpha='50%%'>%s</span>",
                              std::to_string(count).c_str());
                          bottom_label->set_markup(count_str);
                        });
                  },
                  nullptr);
              Telegram::ClientManagerAccessor::handle<
                  td::td_api::chatTypeSupergroup>(
                  chat_type,
                  [this](const std::shared_ptr<td::td_api::chatTypeSupergroup>
                             &supergroup) {
                    // basic_group->ID
                    Telegram::ClientManagerAccessor::send(
                        td::td_api::make_object<
                            td::td_api::getSupergroupFullInfo>(
                            supergroup->supergroup_id_),
                        [this, is_channel = supergroup->is_channel_](
                            const Telegram::ClientManager::SharedObject &obj) {
                          const auto &supergroup_info =
                              Telegram::ClientManager::cast_ptr<
                                  td::td_api::supergroupFullInfo>(obj);
                          std::int32_t count = supergroup_info->member_count_;
                          if (is_channel) {
                            String count_str = GLib::strdup_printf(
                                "<span alpha='50%%'>%'d Subscribers</span>",
                                count);
                            bottom_label->set_markup(count_str.c_str());
                          } else {
                            String count_str = GLib::strdup_printf(
                                "<span alpha='50%%'>%'d Members</span>", count);
                            bottom_label->set_markup(count_str.c_str());
                          }
                        });
                  },
                  nullptr);
            },
            [](const std::shared_ptr<td::td_api::error> &error) {
              g_print("%s\n", error->message_.c_str());
            });
      });
}
String Chat::handle_user_status(
    const std::shared_ptr<td::td_api::UserStatus> &status) {
  String markup = "<span alpha='50%%'>%s</span>";
  String status_str = "";
  switch (status->get_id()) {
  case td::td_api::userStatusOnline::ID: {
    status_str = "Online";
    break;
  }
  case td::td_api::userStatusRecently::ID: {
    status_str = "Last seen recently";
    break;
  }
  case td::td_api::userStatusLastWeek::ID: {
    status_str = "Seen last week";
    break;
  }
  case td::td_api::userStatusLastMonth::ID: {
    status_str = "Seen last month";
    break;
  }
  case td::td_api::userStatusOffline::ID: {
    auto offline = Telegram::ClientManager::cast_ptr<
        td::td_api::UserStatus, td::td_api::userStatusOffline>(status);
    std::int32_t was_online = offline->was_online_;
    auto now = GLib::DateTime::create_now_local();
    auto ts = GLib::DateTime::create_from_unix_local(was_online);
    auto span = now->difference(ts);
    RefPtr<GLib::DateTime> date =
        GLib::DateTime::create_from_unix_local(was_online);
    if ((span / G_TIME_SPAN_DAY) / 7) {
      status_str =
          GLib::strdup_printf("Last seen %s", date->format("%d/%m/%Y").c_str());
    } else if ((span / G_TIME_SPAN_HOUR) > 24) {
      status_str =
          GLib::strdup_printf("Last seen %s", date->format("%a %H:%M").c_str());
    } else if ((span / G_TIME_SPAN_HOUR) < 24) {
      status_str =
          GLib::strdup_printf("Last seen %s", date->format("%H:%M").c_str());
    }
    break;
  }
  case td::td_api::userStatusEmpty::ID:
  default: {
    break;
  }
  }
  String result = GLib::strdup_printf(markup, status_str.c_str());
  return result;
}
void Chat::set_child_id(std::int64_t chat_id) {
  this->chat_id = chat_id;
  set_header_bar();
  set_chat_content();
}
std::int64_t Chat::get_child_id() const { return this->chat_id; }
RefPtr<Chat> Chat::create() { return Object::create<Chat>(); }
} // namespace FlyingPaper::Views
