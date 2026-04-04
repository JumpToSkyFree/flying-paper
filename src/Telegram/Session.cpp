#include <Telegram/Session.hpp>
#include <memory>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>

namespace FlyingPaper::Session {
std::shared_ptr<Session> Session::get() {
  static std::shared_ptr<Session> instance = std::make_shared<Session>();
  return instance;
}
void Session::set_client(
    std::shared_ptr<Telegram::ClientManager> client_manager) {
  this->client = std::move(client_manager);
}
} // namespace FlyingPaper::Session
