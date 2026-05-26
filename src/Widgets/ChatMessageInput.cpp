#include "peel/Gtk/Align.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/TextView.h"
#include "peel/Gtk/Widget.h"
#include <Telegram/Internal/ClientManagerAccessor.hpp>
#include <Widgets/ChatMessageInput.hpp>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>
#include <utility>

using namespace peel;

namespace FlyingPaper::Widgets {
PEEL_CLASS_IMPL(MessageInput, "FlyingPaperMessageInput", Gtk::Widget);
void MessageInput::Class::init() {
  set_layout_manager_type(Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<MessageInput>();
}
void MessageInput::vfunc_dispose() { parent_vfunc_dispose<MessageInput>(); }
inline void MessageInput::init(Class *) {
  container = Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 8);
  container->set_margin_start(8);
  container->set_margin_end(8);
  container->set_margin_bottom(4);
  container->set_margin_top(4);

  entry = Gtk::Entry::create();
  entry->add_css_class("chat-input-text");
  entry->set_editable(true);
  entry->set_hexpand(true);
  entry->set_halign(Gtk::Align::FILL);
  entry->set_placeholder_text("Write a message...");

  send_button = Gtk::Button::create_with_label("Send");
  send_button->add_css_class("chat-send-button");

  send_button->connect_signal("clicked", [this](Gtk::Button *) {
    String message = this->entry->get_text();
    td::td_api::object_ptr<td::td_api::inputMessageText> message_content =
        td::td_api::make_object<td::td_api::inputMessageText>();
    message_content->text_ =
        td::td_api::make_object<td::td_api::formattedText>();
    message_content->text_->text_ = message;
    entry->set_text("");
    Telegram::ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::sendMessage>(
            chat_id, nullptr, nullptr, nullptr, nullptr,
            std::move(message_content)),
        nullptr);
  });

  container->append(entry);
  container->append(send_button);

  set_parent(container);
}
void MessageInput::set_chat_id(std::int64_t chat_id) {
  this->chat_id = chat_id;
}
FloatPtr<MessageInput> MessageInput::create() {
  return ::Object::create<MessageInput>();
}
} // namespace FlyingPaper::Widgets
