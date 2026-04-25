#include "Telegram/Internal/ClientManagerAccessor.hpp"
#include "Telegram/Session.hpp"
#include "Widgets/Avatar.hpp"
#include "glib.h"
#include "peel/Adw/Toast.h"
#include "peel/Adw/ToastOverlay.h"
#include "peel/GLib/DateTime.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Separator.h"
#include "peel/Gtk/Widget.h"
#include "peel/Pango/EllipsizeMode.h"
#include <Widgets/ChatListItem.hpp>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>
#include <utility>

namespace FlyingPaper::Widgets {
PEEL_CLASS_IMPL(ChatListItem, "FlyingPaperChatListItem", Gtk::Widget);
inline void ChatListItem::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<ChatListItem>();
}
inline void ChatListItem::vfunc_dispose() {
  if (container) {
    container->unparent();
    container = nullptr;
  }
  parent_vfunc_dispose<ChatListItem>();
}
void ChatListItem::set_chat_title(peel::String chat_title) {
  if (chat_title_label) {
    std::string _format = std::string("<b>") + chat_title.c_str() + "</b>";
    chat_title_label->set_markup(_format.c_str());
  }
  this->chat_title = chat_title;
}
void ChatListItem::set_chat_content(FloatPtr<Gtk::Widget> widget) {
  if (widget) {
    if (chat_content) {
      chat_content->unparent();
    }
    chat_content = std::move(widget);
    last_chat_sent_content_and_unread_nmessages_count->prepend(
        std::move(chat_content));
  }
}
inline void ChatListItem::init(Class *) {
  auto session = Session::Session::get();
  session->request_context<RefPtr<Adw::ToastOverlay>>(
      "sidebar-toast-overlay", [this](std::shared_ptr<void> _toast_overlay) {
        sidebar_toast_overlay =
            *Session::Session::cast<RefPtr<Adw::ToastOverlay>>(_toast_overlay);
      });

  RefPtr<Gtk::Box> _container =
      Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 10);
  add_css_class("chat-list-item");
  container = _container;
  chat_title_label = Gtk::Label::create(get_chat_title());
  chat_title_label->set_ellipsize(Pango::EllipsizeMode::END);
  last_message = LastMessage::create();

  avatar = Widgets::Avatar::create_from_text(false, "", 48);

  container->append(avatar);

  datetime = Widgets::DateTime::create("%H:%M");
  datetime->set_timestamp(100000);

  chat_title_and_last_message_time_container =
      make_horizontal_container(chat_title_label, datetime);
  last_chat_sent_content_and_unread_nmessages_count =
      make_horizontal_container(nullptr, nullptr);
  last_chat_sent_content_and_unread_nmessages_count->prepend(last_message);

  info_container = make_vertical_container(
      chat_title_and_last_message_time_container,
      last_chat_sent_content_and_unread_nmessages_count);

  container->append(info_container);

  set_parent(container);
}
FloatPtr<Gtk::Box>
ChatListItem::make_horizontal_container(FloatPtr<Gtk::Widget> left,
                                        FloatPtr<Gtk::Widget> right) {
  FloatPtr<Gtk::Box> ret = Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 5);
  FloatPtr<Gtk::Separator> separator =
      Gtk::Separator::create(Gtk::Orientation::HORIZONTAL);
  separator->set_hexpand(true);
  separator->set_opacity(0);
  if (left) {
    ret->append(std::move(left));
    ret->append(std::move(separator));
  }
  if (right) {
    ret->append(std::move(right));
  }
  return ret;
}
FloatPtr<Gtk::Box>
ChatListItem::make_vertical_container(FloatPtr<Gtk::Widget> left,
                                      FloatPtr<Gtk::Widget> right) {
  FloatPtr<Gtk::Box> ret = Gtk::Box::create(Gtk::Orientation::VERTICAL, 8);
  ret->set_margin_bottom(4);
  ret->set_margin_top(4);
  ret->set_margin_start(4);
  ret->set_margin_end(4);
  ret->set_hexpand(true);
  ret->set_hexpand(true);
  if (left) {
    ret->append(std::move(left));
  }
  if (right) {
    ret->append(std::move(right));
  }
  return ret;
}
void ChatListItem::set_avatar_picture(
    const td::td_api::object_ptr<td::td_api::file> &photo) {
  auto session = Session::Session::get();
  auto client = session->get_client();
  client->download_file(
      photo, 1,
      [this](const td::td_api::file &file) {
        RefPtr<Gio::File> image_file =
            Gio::File::create_for_path(file.local_->path_.c_str());
        RefPtr<Gtk::Picture> picture =
            Gtk::Picture::create_for_file(image_file);
        avatar->set_picture(picture);
      },
      nullptr);
}
void ChatListItem::set_timestamp(std::int32_t timestamp) {
  if (datetime) {
    auto now = GLib::DateTime::create_now_local();
    auto ts = GLib::DateTime::create_from_unix_local(timestamp);
    auto span = now->difference(ts);
    if ((span / G_TIME_SPAN_DAY) / 7) {
      datetime->set_format("%d/%m/%Y");
    } else if ((span / G_TIME_SPAN_HOUR) > 24) {
      datetime->set_format("%a %H:%M");
    } else if ((span / G_TIME_SPAN_HOUR) < 24) {
      datetime->set_format("%a %H:%M");
    }

    datetime->set_timestamp(timestamp);
  }
  this->timestamp = timestamp;
}
void ChatListItem::set_avatar_text(const std::string &fullname) {
  avatar->set_text(fullname.c_str());
}
void ChatListItem::set_chat(const std::shared_ptr<td::td_api::chat> &chat) {
  if (chat) {
    this->set_chat_title(chat->title_.c_str());
    if (chat->photo_) {
      this->set_avatar_picture(chat->photo_->small_);
    } else {
      this->set_avatar_text(chat->title_);
    }
    if (chat->last_message_) {
      this->set_timestamp(chat->last_message_->date_);
      if (chat->last_message_->content_) {
        bool is_read = true;
        if (!chat->last_message_->is_outgoing_) {
          is_read =
              chat->last_message_->id_ <= chat->last_read_inbox_message_id_;
        }
        last_message->set_message_content(
            is_read, std::move(chat->last_message_->content_));
      }
    }
    for (const auto &pos : chat->positions_) {
      if (pos->list_->get_id() == td::td_api::chatListMain::ID) {
        this->order = pos->order_;
      }
    }
    this->chat_id = chat->id_;
  }
}
void ChatListItem::set_username(String username) { this->username = username; }
void ChatListItem::set_chat_id(std::int32_t chat_id) {
  this->chat_id = chat_id;
}
void ChatListItem::set_order(std::int64_t order) { this->order = order; }
FloatPtr<ChatListItem> ChatListItem::create() {
  return ::Object::create<ChatListItem>();
}
void ChatListItem::set_new_message(
    const std::shared_ptr<td::td_api::updateNewMessage> &new_message) {
  if (new_message) {
    if (new_message->message_) {
      this->set_timestamp(new_message->message_->date_);
      if (new_message->message_->content_) {
        last_message->set_message_content(
            new_message->message_->is_outgoing_,
            std::move(new_message->message_->content_));
      }
    }
  }
}
void ChatListItem::set_last_message(
    const std::shared_ptr<td::td_api::message> &_last_message) {
  if (_last_message) {
    this->set_timestamp(_last_message->date_);
    if (_last_message->content_) {
      last_message->set_message_content(_last_message->is_outgoing_,
                                        std::move(_last_message->content_));
    }
  }
}
RefPtr<Widgets::LastMessage> ChatListItem::get_last_message() {
  return last_message;
}
} // namespace FlyingPaper::Widgets
