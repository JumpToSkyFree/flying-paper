#include <Telegram/ClientManager.hpp>
#include <Telegram/Internal/ClientManagerImpl.hpp>
#include <functional>
#include <memory>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>

namespace FlyingPaper {
namespace Telegram {
class ClientManagerAccessor {
public:
  using SharedObject = std::shared_ptr<td::td_api::Object>;
  using Request = td::tl_object_ptr<td::td_api::Function>;
  using Callback = std::function<void(const SharedObject &)>;
  using Subscription = std::unordered_map<std::uint64_t, Callback>::iterator;
  static void send(std::shared_ptr<ClientManager> &manager,
                   td::tl_object_ptr<td::td_api::Function> function,
                   Callback callback) {
    if (manager)
      manager->impl->send(std::move(function), callback);
  }
  static std::uint64_t subscribe(std::shared_ptr<ClientManager> &manager,
                                 std::int32_t object_id, Callback callback) {
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
