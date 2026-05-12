#ifndef FLYING_PAPER_SCROLLED_CONTAINER_HPP
#define FLYING_PAPER_SCROLLED_CONTAINER_HPP

#include <functional>
#include <peel/Gtk/ScrolledWindow.h>
#include <peel/class.h>

namespace FlyingPaper::Widgets {
class ScrolledContainer : public peel::Gtk::Widget {
  PEEL_SIMPLE_CLASS(ScrolledContainer, peel::Gtk::Widget)
  inline void init(Class *);
  inline void vfunc_dispose();

  peel::RefPtr<peel::Gtk::ScrolledWindow> scrolled_window;

  double threshold{10.0};
  std::function<void()> _on_treshold_reached;

public:
  static peel::RefPtr<ScrolledContainer> create();
  peel::RefPtr<peel::Gtk::ScrolledWindow> get_scrolled_window();
  void set_threhshold(double threshold);
  double get_threhshold() const;

  void on_threshold_reached_start(std::function<void()> &&threshold);
  void on_threshold_reached_end(std::function<void()> &&threshold);
};
} // namespace FlyingPaper::Widgets

#endif
