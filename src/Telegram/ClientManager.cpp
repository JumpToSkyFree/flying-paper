#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerImpl.hpp>
#include <cstdint>
#include <functional>
#include <memory>

namespace FlyingPaper {
namespace Telegram {
ClientManager::ClientManager()
    : impl(std::make_unique<ClientManager::ClientManagerPrivate>()) {}

ClientManager::ClientManager(const ClientManagerAuthorizationParams &params)
    : impl(std::make_unique<ClientManager::ClientManagerPrivate>()) {
  this->impl->params = params;
}
void ClientManager::set_params(
    const ClientManager::ClientManagerAuthorizationParams &params) {
  this->impl->params = params;
}
ClientManager::ClientManagerAuthorizationParams
ClientManager::get_params() const {
  return this->impl->params;
}
std::unique_ptr<ClientManager>
ClientManager::create(const ClientManagerAuthorizationParams &params) {
  std::unique_ptr<ClientManager> manager(new ClientManager(params));
  return manager;
}
void ClientManager::start_loop() { this->impl->start_loop(); }
void ClientManager::stop_loop() { this->impl->stop_loop(); }
void ClientManager::on_authorization_error(Callback callback) {
  this->impl->handle_authorization_error = callback;
}
void ClientManager::on_authentication(
    std::function<void(std::int32_t)> callback) {
  this->impl->handle_authentication = callback;
}
void ClientManager::authorize() { this->impl->authorize(); }
} // namespace Telegram
} // namespace FlyingPaper
