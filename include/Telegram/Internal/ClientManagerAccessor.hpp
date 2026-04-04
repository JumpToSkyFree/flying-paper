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
      return -1;
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
};
} // namespace Telegram
} // namespace FlyingPaper
