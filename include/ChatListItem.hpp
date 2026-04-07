#ifndef FLYING_PAPER_CHAT_LIST_ITEM_HPP
#define FLYING_PAPER_CHAT_LIST_ITEM_HPP

#include "Avatar.hpp"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Widget.h"
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>

using namespace peel;

namespace FlyingPaper::Widgets {
class ChatListItem final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(ChatListItem, Gtk::Widget)
  inline void init(Class *);
  inline void vfunc_dispose();

  Gtk::Box *container;
  void make_container();
  RefPtr<Widgets::Avatar> avatar;

public:
  static FloatPtr<ChatListItem> create();
};
} // namespace FlyingPaper::Widgets

#endif
