#include "peel/Gtk/Adjustment.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/ScrolledWindow.h"
#include "peel/Gtk/Widget.h"
#include <Widgets/ScrolledContainer.hpp>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

namespace FlyingPaper::Widgets {
PEEL_CLASS_IMPL(ScrolledContainer, "FlyingPaperScrolledContainer", Gtk::Widget);
void ScrolledContainer::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<ScrolledContainer>();
}
inline void ScrolledContainer::vfunc_dispose() {
  if (scrolled_window) {
    scrolled_window->unparent();
    scrolled_window = nullptr;
  }
  parent_vfunc_dispose<ScrolledContainer>();
}
inline void ScrolledContainer::init(Class *) {
  scrolled_window = Gtk::ScrolledWindow::create();

  set_vexpand(true);
  set_parent(scrolled_window);
}
RefPtr<Gtk::ScrolledWindow> ScrolledContainer::get_scrolled_window() {
  return scrolled_window;
}
RefPtr<ScrolledContainer> ScrolledContainer::create() {
  return ::Object::create<ScrolledContainer>();
}
void ScrolledContainer::set_threhshold(double threshold) {
  this->threshold = threshold;
}
double ScrolledContainer::get_threhshold() const { return threshold; }
void ScrolledContainer::on_threshold_reached_start(std::function<void()> &&cb) {
  auto adj = scrolled_window->get_vadjustment();
  this->_on_treshold_reached = std::move(cb);
  adj->connect_signal("value_changed", [adj, this](Gtk::Adjustment *) {
    double current = adj->get_value();
    if (current <= threshold) {
      _on_treshold_reached();
    }
  });
}
void ScrolledContainer::on_threshold_reached_end(std::function<void()> &&cb) {
  auto adj = scrolled_window->get_vadjustment();
  this->_on_treshold_reached = std::move(cb);
  adj->connect_signal("value_changed", [adj, this](Gtk::Adjustment *) {
    double current = adj->get_value();
    double target = adj->get_upper() - adj->get_page_size();

    if (current >= (target - threshold)) {
      _on_treshold_reached();
    }
  });
}
} // namespace FlyingPaper::Widgets
