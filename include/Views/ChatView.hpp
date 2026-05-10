#ifndef FLYING_PAPER_CHAT_VIEW_HPP
#define FLYING_PAPER_CHAT_VIEW_HPP

#include "peel/Adw/HeaderBar.h"
#include "peel/Adw/NavigationPage.h"
#include "peel/Adw/NavigationView.h"
#include "peel/Adw/ToolbarView.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Widget.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <peel/FloatPtr.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/property.h>
#include <td/telegram/td_api.h>

using namespace peel;
namespace FlyingPaper::Views {
class Chat : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(Chat, Gtk::Widget)
  using Callback = std::function<void()>;

private:
  template <typename F> static void define_properties(F &f) {
    f.prop(prop_chat_id(), INT64_MIN, INT64_MAX, 0)
        .get(&Chat::get_child_id)
        .set(&Chat::set_child_id);
  }

  std::int64_t chat_id;
  std::int64_t user_id;
  std::uint64_t update_user_status_sub{0};
  RefPtr<Adw::ToolbarView> toolbar_view;
  RefPtr<Adw::NavigationPage> chat_page;
  RefPtr<Adw::NavigationPage> profile_page;
  RefPtr<Adw::HeaderBar> header_bar;
  RefPtr<Gtk::Label> chat_title;
  RefPtr<Gtk::Label> bottom_label;
  RefPtr<Adw::NavigationView> navigation_view;

  inline void init(Class *);
  inline void vfunc_dispose();

  void make_header_bar();
  String
  handle_user_status(const std::shared_ptr<td::td_api::UserStatus> &status);
  void set_header_bar();
  void set_chat_content();

  PEEL_PROPERTY(std::int64_t, chat_id, "chat-id");

public:
  static RefPtr<Chat> create(std::int64_t);
  void set_child_id(std::int64_t chat_id);
  std::int64_t get_child_id() const;
};
} // namespace FlyingPaper::Views

#endif
