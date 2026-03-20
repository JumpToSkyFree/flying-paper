#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerImpl.hpp>
#include <functional>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>

namespace FlyingPaper {
namespace Telegram {
class ClientManagerAccessor {
public:
  using Object = td::td_api::object_ptr<td::td_api::Object>;
  using Callback = std::function<void(const Object &)>;
  using CustomInvoker = std::function<void(Callback)>;
  static void send(ClientManager &manager,
                   td::tl_object_ptr<td::td_api::Function> function,
                   Callback callback) {
    manager.impl->send(std::move(function), callback);
  }
  static std::uint64_t subscribe(ClientManager &manager, std::int32_t object_id,
                                 Callback callback) {
    return manager.impl->subscribe(object_id, std::move(callback));
  }
  static void unsubscribe(ClientManager &manager, std::int32_t object_id,
                          std::uint64_t subscription_id) {
    manager.impl->unsubscribe(object_id, subscription_id);
  }
};
} // namespace Telegram
} // namespace FlyingPaper
