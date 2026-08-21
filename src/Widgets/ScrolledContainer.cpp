#include "peel/Gtk/Adjustment.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/ScrolledWindow.h"
#include "peel/Gtk/Widget.h"
#include <Widgets/ScrolledContainer.hpp>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

namespace FlyingPaper::Widgets {
using peel::RefPtr;

PEEL_CLASS_IMPL(ScrolledContainer, "FlyingPaperScrolledContainer",
                peel::Gtk::Widget);
void ScrolledContainer::Class::init() {
  set_layout_manager_type(peel::Type::of<peel::Gtk::BinLayout>());
  override_vfunc_dispose<ScrolledContainer>();
}
inline void ScrolledContainer::vfunc_dispose() {
  start_connection.disconnect();
  end_connection.disconnect();
  if (scrolled_window) {
    scrolled_window->unparent();
    scrolled_window = nullptr;
  }
  parent_vfunc_dispose<ScrolledContainer>();
}
inline void ScrolledContainer::init(Class *) {
  scrolled_window = peel::Gtk::ScrolledWindow::create();

  set_vexpand(true);
  set_parent(scrolled_window);
}
peel::RefPtr<peel::Gtk::ScrolledWindow>
ScrolledContainer::get_scrolled_window() {
  return scrolled_window;
}
RefPtr<ScrolledContainer> ScrolledContainer::create() {
  return Object::create<ScrolledContainer>();
}
void ScrolledContainer::set_threhshold(double threshold) {
  this->threshold = threshold;
}
double ScrolledContainer::get_threhshold() const { return threshold; }
void ScrolledContainer::on_threshold_reached_start(std::function<void()> &&cb) {
  auto adj = scrolled_window->get_vadjustment();
  this->on_threshold_start = std::move(cb);
  start_connection = adj->connect_signal(
      "value_changed",
      [self = peel::WeakPtr<ScrolledContainer>(this),
       adj](peel::Gtk::Adjustment *) {
        if (!self)
          return;
        double current = adj->get_value();
        if (current <= self->threshold)
          self->on_threshold_start();
      });
}
void ScrolledContainer::on_threshold_reached_end(std::function<void()> &&cb) {
  auto adj = scrolled_window->get_vadjustment();
  this->on_threshold_end = std::move(cb);
  end_connection = adj->connect_signal(
      "value_changed",
      [self = peel::WeakPtr<ScrolledContainer>(this),
       adj](peel::Gtk::Adjustment *) {
        if (!self)
          return;
        double current = adj->get_value();
        double target = adj->get_upper() - adj->get_page_size();

        if (current >= (target - self->threshold))
          self->on_threshold_end();
      });
}
} // namespace FlyingPaper::Widgets
