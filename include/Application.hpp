#include "Telegram/ClientManager.hpp"
#include "peel/Gio/Application.h"
#include "peel/Gio/SimpleAction.h"
#include <memory>
#include <peel/Adw/Application.h>
#include <peel/GLib/Variant.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

using namespace peel;

namespace FlyingPaper {
namespace Application {
class Application final : public Adw::Application {
  PEEL_SIMPLE_CLASS(Application, Adw::Application)
  friend class Gio::Application;
  inline void init(Class *);
  inline void vfunc_activate();
  inline void vfunc_dispose();
  inline void vfunc_startup();
  void action_quit(Gio::SimpleAction *, GLib::Variant *);
  void action_about(Gio::SimpleAction *, GLib::Variant *);
  std::shared_ptr<Telegram::ClientManager> client_manager;

public:
  static RefPtr<Application> create();
};
} // namespace Application
} // namespace FlyingPaper
