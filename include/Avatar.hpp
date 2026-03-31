#ifndef FLYING_PAPER_AVATAR_HPP
#define FLYING_PAPER_AVATAR_HPP
#include "glib.h"
#include "peel/Adw/Avatar.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Picture.h"
#include "peel/Gtk/Widget.h"
#include <peel/FloatPtr.h>
#include <peel/GObject/Type.h>
#include <peel/Gtk/Gtk.h>
#include <peel/RefPtr.h>
#include <peel/String.h>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/signal.h>
using namespace peel;
namespace FlyingPaper::Widgets {
class Avatar final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(Avatar, Gtk::Widget)

  inline void init(Class *);
  inline void vfunc_dispose();

  RefPtr<Gtk::Button> avatar_clickable;
  RefPtr<Adw::Avatar> avatar;

  void make_avatar_clickable_button();

  template <typename F> static void define_properties(F &f) {
    f.prop(prop_clickable(), 0, 1, 0)
        .get(&Avatar::get_clickable)
        .set(&Avatar::set_clickable);
    f.prop(prop_avatar_size(), 0, 512, 32)
        .get(&Avatar::get_avatar_size)
        .set(&Avatar::set_avatar_size);
    f.prop(prop_text(), "").get(&Avatar::get_text).set(&Avatar::set_text);
  }

  gboolean clickable;
  guint avatar_size;
  peel::String text;

  PEEL_PROPERTY(gboolean, clickable, "clickable");
  PEEL_PROPERTY(guint, avatar_size, "avatar-size");
  PEEL_PROPERTY(peel::String, text, "text");

  static Signal<Avatar, void(void)> sig_avatar_clicked;
  PEEL_SIGNAL_CONNECT_METHOD(avatar_clicked, sig_avatar_clicked);

public:
  static RefPtr<Avatar> create_from_text(gboolean clickable, peel::String text,
                                         guint avatar_size);

  gboolean get_clickable() const { return clickable; }
  void set_clickable(gboolean clickable);

  guint get_avatar_size() const { return avatar_size; }
  void set_avatar_size(guint size);

  peel::String get_text() const { return text; }
  void set_text(peel::String text);

  Adw::Avatar *get_avatar() const { return avatar; }

  void set_picture(RefPtr<Gtk::Picture>);
};
} // namespace FlyingPaper::Widgets

#endif
