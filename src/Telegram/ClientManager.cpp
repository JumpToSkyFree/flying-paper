#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerImpl.hpp>
#include <memory>

namespace FlyingPaper {
namespace Telegram {
ClientManager::ClientManager()
    : impl(std::make_unique<ClientManager::ClientManagerPrivate>()) {}

ClientManager::ClientManager(const ClientManagerAuthorizationParams &params)
    : impl(std::make_unique<ClientManager::ClientManagerPrivate>()) {
  this->params = params;
}
ClientManager::~ClientManager() {}
void ClientManager::set_params(
    ClientManager::ClientManagerAuthorizationParams &params) {
  this->params = params;
}
ClientManager::ClientManagerAuthorizationParams
ClientManager::get_params() const {
  return this->params;
}
std::unique_ptr<ClientManager>
ClientManager::create(const ClientManagerAuthorizationParams &params) {
  std::unique_ptr<ClientManager> manager(new ClientManager(params));
  return manager;
}
void ClientManager::start_loop() { this->impl->start_loop(); }
void ClientManager::stop_loop() { this->impl->stop_loop(); }
} // namespace Telegram
} // namespace FlyingPaper
