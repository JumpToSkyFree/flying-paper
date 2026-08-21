#include "Telegram/Session.hpp"
#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerImpl.hpp>
#include <memory>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>

namespace FlyingPaper {
namespace Telegram {
class ClientManagerAccessor {
public:
  static void send(td::tl_object_ptr<td::td_api::Function> function,
                   ClientManager::Callback callback) {
    auto session = Session::Session::get();
    auto client = session->get_client();
    if (!client)
      return;
    client->impl->send(std::move(function), callback);
  }
  static std::uint64_t subscribe(std::int32_t object_id,
                                 ClientManager::Callback callback) {
    auto session = Session::Session::get();
    auto client = session->get_client();
    if (!client)
      return 0;
    return client->impl->subscribe(object_id, std::move(callback));
  }
  static void unsubscribe(std::int32_t object_id,
                          std::uint64_t subscription_id) {
    auto session = Session::Session::get();
    auto client = session->get_client();
    if (!client)
      return;
    client->impl->unsubscribe(object_id, subscription_id);
  }
  template <typename T = td::td_api::Object>
  static void
  handle(const ClientManager::SharedObject &obj,
         std::function<void(const std::shared_ptr<T> &)> on_success,
         std::function<void(const std::shared_ptr<td::td_api::error> &)>
             on_error) {
    if (on_error && obj->get_id() == td::td_api::error::ID) {
      on_error(
          ClientManager::cast_ptr<td::td_api::Object, td::td_api::error>(obj));
      return;
    }
    if (on_success && obj->get_id() == T::ID) {
      on_success(ClientManager::cast_ptr<td::td_api::Object, T>(obj));
      return;
    }
  }
  template <typename U, typename T>
  static void
  handle(const std::shared_ptr<U> &obj,
         std::function<void(const std::shared_ptr<T> &)> on_success,
         std::function<void(const std::shared_ptr<td::td_api::error> &)>
             on_error) {
    if (on_error && obj->get_id() == td::td_api::error::ID) {
      std::shared_ptr<td::td_api::Object> _obj =
          std::static_pointer_cast<td::td_api::Object>(obj);
      on_error(
          ClientManager::cast_ptr<td::td_api::Object, td::td_api::error>(_obj));
      return;
    }
    if (on_success && obj->get_id() == T::ID) {
      on_success(ClientManager::cast_ptr<U, T>(obj));
      return;
    }
  }
  template <typename U, typename T>
  static void
  handle(const U *obj,
         std::function<void(const std::shared_ptr<T> &)> on_success,
         std::function<void(const std::shared_ptr<td::td_api::error> &)>
             on_error) {
    if (on_error && obj->get_id() == td::td_api::error::ID) {
      std::shared_ptr<td::td_api::Object> _obj =
          std::static_pointer_cast<td::td_api::Object>(obj);
      on_error(
          ClientManager::cast_ptr<td::td_api::Object, td::td_api::error>(_obj));
      return;
    }
    if (on_success && obj->get_id() == T::ID) {
      on_success(ClientManager::cast_ptr<U, T>(obj));
      return;
    }
  }
};
} // namespace Telegram
} // namespace FlyingPaper
