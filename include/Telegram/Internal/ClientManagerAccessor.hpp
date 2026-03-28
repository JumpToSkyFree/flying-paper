#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerImpl.hpp>
#include <memory>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>

namespace FlyingPaper {
namespace Telegram {
class ClientManagerAccessor {
public:
  static void send(std::shared_ptr<ClientManager> &manager,
                   td::tl_object_ptr<td::td_api::Function> function,
                   ClientManager::Callback callback) {
    if (manager)
      manager->impl->send(std::move(function), callback);
  }
  static std::uint64_t subscribe(std::shared_ptr<ClientManager> &manager,
                                 std::int32_t object_id,
                                 ClientManager::Callback callback) {
    if (manager)
      return manager->impl->subscribe(object_id, std::move(callback));
    return -1;
  }
  static void unsubscribe(std::shared_ptr<ClientManager> &manager,
                          std::int32_t object_id,
                          std::uint64_t subscription_id) {
    if (manager)
      manager->impl->unsubscribe(object_id, subscription_id);
  }
};
} // namespace Telegram
} // namespace FlyingPaper
