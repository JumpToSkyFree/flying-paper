#include "Telegram/ClientManager.hpp"
#include "Telegram/Internal/ClientManagerAccessor.hpp"
#include "Telegram/Session.hpp"
#include "Views/ChatView.hpp"
#include "Widgets/Avatar.hpp"
#include "Widgets/DateTime.hpp"
#include "glib.h"
#include "peel/GLib/functions.h"
#include "peel/Gtk/Align.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Picture.h"
#include "peel/Gtk/TextView.h"
#include "peel/Gtk/Viewport.h"
#include "peel/Gtk/Widget.h"
#include "peel/Gtk/WrapMode.h"
#include <Widgets/ChatMessage.hpp>
#include <cstdint>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/Pango/WrapMode.h>
#include <peel/WeakPtr.h>
#include <peel/class.h>
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <utility>

using namespace peel;

namespace FlyingPaper::Widgets {
PEEL_CLASS_IMPL(ChatMessage, "FlyingPaperChatMessage", Gtk::Widget);
void ChatMessage::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<ChatMessage>();
}
inline void ChatMessage::vfunc_dispose() {
  if (container) {
    container->unparent();
    container = nullptr;
  }
  parent_vfunc_dispose<ChatMessage>();
}
inline void ChatMessage::init(Class *) {
  container = Gtk::Box::create(Gtk::Orientation::VERTICAL, 0);
  set_parent(container);
}
FloatPtr<ChatMessage> ChatMessage::create() {
  return ::Object::create<ChatMessage>();
}
void ChatMessage::set_is_group(bool is_group) { this->is_group = is_group; }
void ChatMessage::set_message_content(
    std::shared_ptr<td::td_api::message> owned_message) {
  if (!owned_message || !owned_message->content_)
    return;

  this->message = std::move(owned_message);
  td::td_api::message *message = this->message.get();
  td::td_api::MessageContent *message_content = message->content_.get();
  this->message_id = message->id_;
  this->message_container = create_message_container();
  this->message_container->set_margin_start(8);
  this->message_container->set_margin_end(8);

  bubble_container = Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 8);
  message_container->append(bubble_container);
  bubble_container->add_css_class("bubble");

  if (this->message->is_outgoing_) {
    bubble_container->add_css_class("outgoing");
    bubble_container->set_halign(Gtk::Align::END);
  } else {
    bubble_container->add_css_class("ingoing");
    bubble_container->set_halign(Gtk::Align::START);
  }

  if (message->sender_id_ &&
      message->sender_id_->get_id() == td::td_api::messageSenderUser::ID) {
    auto sender =
        static_cast<td::td_api::messageSenderUser *>(&*message->sender_id_);
    peel::WeakPtr<ChatMessage> self_weak(this);
    Telegram::ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::getUser>(sender->user_id_),
        [self_weak](const Telegram::ClientManager::SharedObject &obj) {
          if (!self_weak || obj->get_id() != td::td_api::user::ID)
            return;
          ChatMessage *self = self_weak;
          Telegram::ClientManagerAccessor::handle<td::td_api::user>(
              obj,
              [self](const std::shared_ptr<td::td_api::user> &user) {
                auto session = Session::Session::get();
                auto client = session->get_client();
                std::string username;
                if (user->usernames_) {
                  if (user->usernames_->active_usernames_.size()) {
                    username = user->usernames_->active_usernames_.back();
                  }
                }
                if (self->is_group) {
                  // TODO: Make a seperate container for message text and user
                  // username. String user_name_markup = GLib::strdup_printf(
                  //     "<b>%s</b> (<span alpha='50%%'>%s</span>)",
                  //     user->first_name_.c_str(), username.c_str());
                  // FloatPtr<Gtk::Label> username_label =
                  // Gtk::Label::create("");
                  // username_label->set_markup(user_name_markup.c_str());
                  // username_label->set_halign(Gtk::Align::START);
                  // bubble_container->prepend(std::move(username_label));
                }
                if (user->profile_photo_ && user->profile_photo_->small_) {
                  peel::WeakPtr<ChatMessage> owner(self);
                  client->download_file(
                      user->profile_photo_->small_, 1,
                      [owner](const td::td_api::file &file) {
                        if (!owner)
                          return;
                        ChatMessage *self = owner;
                        auto pic_file = Gtk::Picture::create_for_filename(
                            file.local_->path_.c_str());
                        if (self->is_group) {
                          auto avatar = Widgets::Avatar::create_from_picture(
                              false, "", pic_file, 16);
                          self->message_container->prepend(avatar);
                        }
                      },
                      {/* TODO: Show failure message. */});
                }
              },
              nullptr);
        });
  }

  switch (message_content->get_id()) {
  case td::td_api::messageText::ID: {
    auto message_text = static_cast<td::td_api::messageText *>(message_content);
    auto label = create_label();
    auto datetime = DateTime::create("%H:%M");
    datetime->set_timestamp(message->date_);
    datetime->set_valign(Gtk::Align::END);
    auto buffer = label->get_buffer();
    buffer->set_text(message_text->text_->text_.c_str(),
                     message_text->text_->text_.size());
    bubble_container->append(std::move(label));
    bubble_container->append(std::move(datetime));
    break;
  }
  case td::td_api::messagePhoto::ID: {
    break;
  }
  default: {
    auto label = create_label();
    auto buffer = label->get_buffer();
    std::string message_not_supported = "Message content not supported";
    buffer->set_text(message_not_supported.c_str(),
                     message_not_supported.size());
    label->set_opacity(0.5);
    bubble_container->append(std::move(label));
  }
  }

  container->append(message_container);
}
RefPtr<Gtk::Box> ChatMessage::create_message_container() {
  FloatPtr<Gtk::Box> ret = Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 0);
  ret->add_css_class("message-container");
  ret->set_hexpand(false);
  return ret;
}
FloatPtr<Gtk::TextView> ChatMessage::create_label() {
  FloatPtr<Gtk::TextView> text_view = Gtk::TextView::create();
  text_view->set_size_request(300, -1);
  text_view->set_editable(false);
  text_view->set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
  text_view->set_hexpand(true);
  text_view->add_css_class("message-label");
  text_view->set_cursor_visible(false);
  return text_view;
}
std::int64_t ChatMessage::get_id() { return this->message_id; }
td::td_api::message *ChatMessage::get_message_obj() { return message.get(); }
} // namespace FlyingPaper::Widgets
