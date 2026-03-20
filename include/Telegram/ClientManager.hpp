#ifndef FLYING_PAPER_TELEGRAM_MANAGER_HPP
#define FLYING_PAPER_TELEGRAM_MANAGER_HPP

#include <cstdint>
#include <memory>
#include <string>

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
  ClientManagerAuthorizationParams params;
  friend class ClientManagerAccessor;

public:
  static std::unique_ptr<ClientManager>
  create(const ClientManagerAuthorizationParams &params);

  ClientManager(const ClientManager &) = delete;
  ClientManager &operator=(const ClientManager &) = delete;
  ClientManager(ClientManager &&) noexcept;
  ClientManager &operator=(ClientManager &&) noexcept;
  ~ClientManager();

  void set_params(ClientManagerAuthorizationParams &params);
  ClientManagerAuthorizationParams get_params() const;

  bool is_running() const;
  void start_loop();
  void stop_loop();
};
} // namespace Telegram
} // namespace FlyingPaper

#endif // !FLYING_PAPER_TELEGRAM_MANAGER_HPP
