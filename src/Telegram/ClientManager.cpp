#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerAccessor.hpp>
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
void ClientManager::download_file(
    const td::td_api::object_ptr<td::td_api::file> &file, std::int32_t priority,
    std::function<void(const td::td_api::file &)> on_success,
    std::function<void(const td::td_api::error &)> on_error) {
  const std::int32_t file_id = file->id_;
  if (file->local_ && file->local_->is_downloading_completed_) {
    on_success(*file);
    return;
  } else if (file->remote_ && file->remote_->id_ != "0") {
    ClientManagerAccessor::send(
        td::td_api::make_object<td::td_api::downloadFile>(file->id_, priority,
                                                          0, 0, false),
        [on_error](const SharedObject &obj) {
          if (obj->get_id() == td::td_api::error::ID) {
            if (on_error) {
              on_error(ClientManager::cast<td::td_api::error>(obj));
            }
          }
        });
    auto sub_id = std::make_shared<std::uint64_t>(0);
    *sub_id = ClientManagerAccessor::subscribe(
        td::td_api::updateFile::ID,
        [on_success, on_error, sub_id, file_id](const SharedObject &obj) {
          ClientManagerAccessor::handle<td::td_api::updateFile>(
              obj,
              [&on_success, sub_id, file_id](
                  const std::shared_ptr<td::td_api::updateFile> &update_file) {
                if (update_file->file_->id_ == file_id) {
                  if (update_file->file_->local_ &&
                      update_file->file_->local_->is_downloading_completed_) {
                    on_success(*update_file->file_);
                    if (*sub_id != 0) {
                      ClientManagerAccessor::unsubscribe(
                          td::td_api::updateFile::ID, *sub_id);
                      *sub_id = 0;
                    }
                  }
                }
              },
              [&on_error,
               sub_id](const std::shared_ptr<td::td_api::error> &error) {
                if (on_error) {
                  on_error(*error);
                }
                if (*sub_id != 0) {
                  ClientManagerAccessor::unsubscribe(td::td_api::updateFile::ID,
                                                     *sub_id);
                  *sub_id = 0;
                }
              });
        });
  }
}
} // namespace Telegram
} // namespace FlyingPaper
