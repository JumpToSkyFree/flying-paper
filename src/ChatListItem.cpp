#include "Telegram/Internal/ClientManagerAccessor.hpp"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Widget.h"
#include <ChatListItem.hpp>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

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
inline void ChatListItem::init(Class *) {
  RefPtr<Gtk::Box> _container =
      Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 32);
  add_css_class("chat-list-item");
  container = _container;
  set_parent(container);
}
void ChatListItem::make_container() {}
FloatPtr<ChatListItem> ChatListItem::create() {
  return ::Object::create<ChatListItem>();
}
} // namespace FlyingPaper::Widgets
