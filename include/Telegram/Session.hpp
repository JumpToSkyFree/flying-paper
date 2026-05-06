#ifndef FLYING_PAPER_SESSION_HPP
#define FLYING_PAPER_SESSION_HPP

#include "Telegram/ClientManager.hpp"
#include "glib.h"
#include "peel/GLib/functions.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <td/telegram/td_api.h>
#include <unordered_map>

namespace FlyingPaper::Session {
class Session {
public:
  using Request = std::function<void(std::shared_ptr<void>)>;
  using RequestHandle = std::uint64_t;

private:
  std::shared_ptr<Telegram::ClientManager> client;
  std::unordered_map<std::string, std::shared_ptr<void>> contexts;
  std::unordered_map<std::string,
                     std::vector<std::pair<RequestHandle, Request>>>
      pending_requests;
  RequestHandle next_request_id{0};

  std::mutex registry_mtx;

public:
  static std::shared_ptr<Session> get();
  const std::shared_ptr<Telegram::ClientManager> &get_client() const {
    return client;
  }
  void set_client(std::shared_ptr<Telegram::ClientManager> client_manager);
  template <typename T, template <typename U> class Ptr>
  void set_context(std::string_view name, Ptr<T> ctx);

  template <typename T>
  std::shared_ptr<T> set_context(std::string_view name, T &ctx) {
    auto ptr = std::make_shared<T>(ctx);
    std::lock_guard<std::mutex> lock(registry_mtx);

    std::string key(name);

    contexts[key] = ptr;
    if (pending_requests.contains(key)) {
      for (auto &pair : pending_requests[key]) {
        peel::GLib::idle_add([_ptr = ptr, _task = pair.second]() {
          _task(_ptr);
          return G_SOURCE_REMOVE;
        });
      }
      pending_requests.erase(key);
    }

    return ptr;
  }

  template <typename T>
  void set_context(std::string_view name, std::shared_ptr<T> ctx) {
    std::lock_guard<std::mutex> lock(registry_mtx);

    std::string key(name);

    contexts[key] = ctx;

    if (pending_requests.contains(key)) {
      for (auto &pair : pending_requests[key]) {
        peel::GLib::idle_add([_ctx = ctx, _task = pair.second]() {
          _task(_ctx);
          return G_SOURCE_REMOVE;
        });
      }
      pending_requests.erase(key);
    }
  }
  template <typename T>
  static std::shared_ptr<T> cast(std::shared_ptr<void> &obj) {
    return std::static_pointer_cast<T>(obj);
  }
  template <typename T>
  RequestHandle request_context(std::string name, Request request) {
    std::lock_guard<std::mutex> lock(registry_mtx);

    RequestHandle request_id = next_request_id++;

    if (contexts.contains(name)) {
      auto ctx = contexts[name];
      peel::GLib::idle_add([_ctx = ctx, request]() {
        request(_ctx);
        return G_SOURCE_REMOVE;
      });
      return request_id;
    }

    pending_requests[name].push_back({request_id, request});

    return request_id;
  }
  void cancel_request(const std::string &name, RequestHandle id) {
    std::lock_guard<std::mutex> lock(registry_mtx);
    auto it = pending_requests.find(name);
    if (it == pending_requests.end())
      return;
    auto &vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
                             [id](const auto &p) { return p.first == id; }),
              vec.end());
  }
};
} // namespace FlyingPaper::Session

#endif
