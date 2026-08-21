#ifndef FLYING_PAPER_TELEGRAM_MANAGER_HPP
#define FLYING_PAPER_TELEGRAM_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <td/telegram/td_api.h>

namespace FlyingPaper {
namespace Telegram {

class ClientManager {
public:
  struct ClientManagerAuthorizationParams {
    std::int32_t api_id;
    std::string api_hash;
    std::string system_language;
    std::string application_version;
  };

private:
  struct ClientManagerPrivate;
  std::unique_ptr<ClientManagerPrivate> impl;

  explicit ClientManager(const ClientManagerAuthorizationParams &params);
  ClientManager();
  friend class ClientManagerAccessor;

public:
  using Request = td::tl_object_ptr<td::td_api::Function>;
  using SharedObject = std::shared_ptr<td::td_api::Object>;
  using Callback = std::function<void(const SharedObject &)>;
  using Subscription = std::unordered_map<std::uint64_t, Callback>::iterator;

  static std::unique_ptr<ClientManager>
  create(const ClientManagerAuthorizationParams &params);

  ClientManager(const ClientManager &) = delete;
  ClientManager &operator=(const ClientManager &) = delete;
  ClientManager(ClientManager &&) noexcept;
  ClientManager &operator=(ClientManager &&) noexcept;
  ~ClientManager() = default;

  void set_params(const ClientManagerAuthorizationParams &params);
  ClientManagerAuthorizationParams get_params() const;

  bool is_running() const;
  void start_loop();
  void stop_loop();
  void authorize();
  void on_authorization_error(Callback callback);
  void on_authentication(std::function<void(std::int32_t)> callback);

  template <typename T> static const T &cast(const SharedObject &obj) {
    return static_cast<const T &>(*obj);
  }

  template <typename T>
  static std::shared_ptr<T> cast_ptr(const SharedObject &obj) {
    return std::static_pointer_cast<T>(obj);
  }

  template <typename U, typename T>
  static std::shared_ptr<T> cast_ptr(const std::shared_ptr<U> &obj) {
    return std::static_pointer_cast<T>(obj);
  }

  template <typename U, typename T>
  static std::shared_ptr<T> cast_ptr(td::td_api::object_ptr<U> obj) {
    return std::shared_ptr<T>(static_cast<T *>(obj.release()));
  }

  template <typename U, typename T> static T *cast_ptr(U *obj) {
    return static_cast<T *>(obj);
  }

  void download_file(const td::td_api::object_ptr<td::td_api::file> &file,
                     std::int32_t priority,
                     std::function<void(const td::td_api::file &)> on_success,
                     std::function<void(const td::td_api::error &)> on_error);
};
} // namespace Telegram
} // namespace FlyingPaper

#endif // !FLYING_PAPER_TELEGRAM_MANAGER_HPP
