#ifndef FLYING_PAPER_VERIFICATION_CODE_INSERT_VIEW_HPP
#define FLYING_PAPER_VERIFICATION_CODE_INSERT_VIEW_HPP
#include "Telegram/ClientManager.hpp"
#include "peel/Adw/StatusPage.h"
#include "peel/Gtk/Box.h"
#include "peel/Gtk/Button.h"
#include "peel/Gtk/Entry.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/ShortcutController.h"
#include "peel/Gtk/Widget.h"
#include <cstdint>
#include <memory>
#include <peel/Adw/Adw.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/signal.h>
#include <vector>
using namespace peel;
namespace FlyingPaper::Views {
class VerificationCodeInsertView final : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(VerificationCodeInsertView, Gtk::Widget)
  friend class Gtk::Widget;

  inline void init(Class *);
  inline void vfunc_dispose();
  void setup_entry(Gtk::Entry *);

  template <typename F> static void define_properties(F &f) {
    f.prop(prop_n_entries(), 0, 0, 0)
        .get(&VerificationCodeInsertView::get_n_entries)
        .set(&VerificationCodeInsertView::set_n_entries);
  }

  std::shared_ptr<Telegram::ClientManager> client_manager;
  RefPtr<Gtk::Box> inputs_box;
  Adw::StatusPage *status_page;
  std::vector<Gtk::Entry *> entries;
  Gdk::Clipboard *clipboard;
  std::uint32_t n_entries;
  RefPtr<Gtk::ShortcutController> controller;
  RefPtr<Gtk::Shortcut> shortcut;
  std::uint32_t countdown{300000};
  std::uint64_t timeout_id{0};

  RefPtr<Gtk::Button> send_again;

  Adw::ViewStack *view_stack;

  RefPtr<Gtk::Button> verify_button;

  void handle_clipboard_paste();
  FloatPtr<Gtk::Button> resend_countdown_box();

  static Signal<VerificationCodeInsertView, void(void)> sig_continue_clicked;

public:
  static FloatPtr<VerificationCodeInsertView>
  create(std::shared_ptr<Telegram::ClientManager> client_manager);
  void setup();
  void set_n_entries(std::uint32_t entries);
  std::uint32_t get_n_entries() const { return n_entries; }
  std::string get_verification_code() const;

  void set_view_stack(Adw::ViewStack *view_stack);

  PEEL_PROPERTY(std::uint32_t, n_entries, "n-entries");
  PEEL_SIGNAL_CONNECT_METHOD(continue_clicked, sig_continue_clicked);
};
} // namespace FlyingPaper::Views
#endif
