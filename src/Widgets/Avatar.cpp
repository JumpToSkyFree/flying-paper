#include "peel/Adw/Avatar.h"
#include "glib.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/LayoutManager.h"
#include "peel/Gtk/Picture.h"
#include "peel/Gtk/Widget.h"
#include <Widgets/Avatar.hpp>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <peel/signal.h>

namespace FlyingPaper::Widgets {
// TODO: make the avatar play animated videos.
PEEL_CLASS_IMPL(Avatar, "FlyingPaperAvatar", Gtk::Widget);
void Avatar::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<Avatar>();

  sig_avatar_clicked = Signal<Avatar, void(void)>::create("avatar-clicked");
}
Signal<Avatar, void(void)> Avatar::sig_avatar_clicked;
inline void Avatar::init(Class *) {
  set_name("clickable-avatar");
  avatar = Adw::Avatar::create(get_avatar_size(), text.c_str(), true);
  if (clickable) {
    make_avatar_clickable_button();
    set_parent(avatar_clickable);
  } else {
    set_parent(avatar);
  }
}
inline void Avatar::vfunc_dispose() {
  if (clickable) {
    if (avatar_clickable) {
      avatar_clickable->unparent();
      avatar_clickable = nullptr;
    }
  } else {
    if (avatar) {
      avatar->unparent();
      avatar = nullptr;
    }
  }
  parent_vfunc_dispose<Avatar>();
}
RefPtr<Avatar> Avatar::create_from_text(gboolean clickable, peel::String text,
                                        guint avatar_size) {
  FloatPtr<Avatar> ret =
      ::Object::create<Avatar>(prop_clickable(), clickable, prop_avatar_size(),
                               avatar_size, prop_text(), text);
  return ret;
}
RefPtr<Avatar> Avatar::create_from_picture(gboolean clickable,
                                           peel::String text,
                                           RefPtr<Gtk::Picture> picture,
                                           guint avatar_size) {

  FloatPtr<Avatar> ret =
      ::Object::create<Avatar>(prop_clickable(), clickable, prop_avatar_size(),
                               avatar_size, prop_text(), text);
  ret->set_picture(picture);
  return ret;
}
void Avatar::make_avatar_clickable_button() {
  avatar_clickable = Gtk::Button::create();
  avatar_clickable->add_css_class("rounded");
  avatar_clickable->add_css_class("flat");
  avatar_clickable->set_child(avatar);
  avatar_clickable->connect_signal(
      "clicked", [this](Gtk::Button *) { sig_avatar_clicked.emit(this); });
}
void Avatar::set_avatar_size(guint size) {
  if (!avatar)
    return;
  if (clickable) {
    // NOTE: set the avatar_clickable size.
  }
  avatar->set_size(size);
  this->avatar_size = size;
}
void Avatar::set_text(peel::String text) {
  if (!avatar)
    return;
  avatar->set_text(text.c_str());
  this->text = std::move(text);
}
void Avatar::set_clickable(gboolean clickable) {
  if (this->clickable == clickable)
    return;

  this->clickable = clickable;
  if (!avatar)
    return;

  // Unparent the current setup
  avatar->unparent();
  if (avatar_clickable) {
    avatar_clickable->unparent();
    avatar_clickable = nullptr;
  }

  // Re-build based on new state
  if (clickable) {
    make_avatar_clickable_button();
    set_parent(avatar_clickable);
  } else {
    set_parent(avatar);
  }
}
void Avatar::set_picture(RefPtr<Gtk::Picture> picture) {
  if (avatar) {
    avatar->set_custom_image(picture->get_paintable());
  }
}
} // namespace FlyingPaper::Widgets
