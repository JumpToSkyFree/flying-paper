#ifndef FLYING_PAPER_SCROLLED_CONTAINER_HPP
#define FLYING_PAPER_SCROLLED_CONTAINER_HPP

#include <functional>
#include <peel/Gtk/ScrolledWindow.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

using namespace peel;

namespace FlyingPaper::Widgets {
class ScrolledContainer : public Gtk::Widget {
  PEEL_SIMPLE_CLASS(ScrolledContainer, Gtk::Widget)
  inline void init(Class *);
  inline void vfunc_dispose();

  RefPtr<Gtk::ScrolledWindow> scrolled_window;

  double threshold{10.0};
  std::function<void()> _on_treshold_reached;

public:
  static RefPtr<ScrolledContainer> create();
  RefPtr<Gtk::ScrolledWindow> get_scrolled_window();
  void set_threhshold(double threshold);
  double get_threhshold() const;

  void on_threshold_reached(std::function<void()> &&threshold);
};
} // namespace FlyingPaper::Widgets

#endif
