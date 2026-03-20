#ifndef FLYING_PAPER_CLIENT_MANAGER_IMPL_HPP
#define FLYING_PAPER_CLIENT_MANAGER_IMPL_HPP

#include "Telegram/ClientManager.hpp"
#include "glib.h"
#include "peel/GLib/functions.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>
#include <thread>
#include <unordered_map>

namespace FlyingPaper {
namespace Telegram {
struct ClientManager::ClientManagerPrivate {
  using Object = td::td_api::object_ptr<td::td_api::Object>;
  using Request = td::tl_object_ptr<td::td_api::Function>;
  using Callback = std::function<void(const Object &)>;
  using Subscription = std::unordered_map<std::uint64_t, Callback>::iterator;

  std::unique_ptr<td::ClientManager> client_manager;

  std::atomic_bool is_running{false};
  std::atomic_bool keep_running{true};

  std::int32_t client_id{0};
  std::uint64_t query_id_counter{1};
  std::uint64_t current_query_id{0};
  // std::uint64_t authentication_query_id{0};

  std::mutex handlers_mtx;
  std::mutex subscribers_mtx;

  std::thread worker_thread;

  std::unordered_map<std::int32_t, std::unordered_map<std::uint64_t, Callback>>
      subscribers;
  std::uint64_t next_subscribtion_id{1};

  std::unordered_map<std::uint64_t, Callback> handlers;

  ClientManagerPrivate() {
    client_manager = std::make_unique<td::ClientManager>();
    client_id = client_manager->create_client_id();
    this->send(td::td_api::make_object<td::td_api::getOption>("version"), {});
  }

  ~ClientManagerPrivate() { stop_loop(); }
  void send(Request request, Callback callback) {
    std::uint64_t id = ++this->query_id_counter;
    if (callback) {
      std::lock_guard<std::mutex> lock(handlers_mtx);
      this->handlers.emplace(id, std::move(callback));
    }
    this->client_manager->send(this->client_id, id, std::move(request));
  }
  std::uint64_t subscribe(std::int32_t object_id, Callback callback) {
    std::uint64_t subscribtion_id = ++this->next_subscribtion_id;
    this->subscribers[object_id][subscribtion_id] = callback;
    return subscribtion_id;
  }
  void unsubscribe(std::int32_t object_id, std::uint64_t subscription_id) {
    auto it = this->subscribers.find(object_id);
    if (it != this->subscribers.end()) {
      it->second.erase(subscription_id);

      if (it->second.empty()) {
        this->subscribers.erase(it);
      }
    }
  }
  void loop() {
    while (this->keep_running.load()) {
      const td::ClientManager::Response response =
          this->client_manager->receive(10.0);
      if (!response.object)
        continue;
      if (response.request_id == 0) {
        const std::int32_t type_id = response.object->get_id();
        std::lock_guard<std::mutex> lock(subscribers_mtx);
        auto it_map = this->subscribers.find(type_id);

        if (it_map != this->subscribers.end()) {
          for (auto const &[id, cb] : it_map->second) {
            // cb(response.object);
            peel::GLib::idle_add([&cb, &response]() {
              cb(response.object);
              return G_SOURCE_REMOVE;
            });
          }
        }
      } else {
        std::unordered_map<std::uint64_t, Callback>::iterator it =
            handlers.find(response.request_id);
        std::lock_guard<std::mutex> lock(handlers_mtx);
        if (it != handlers.end()) {
          peel::GLib::idle_add([this, &it, &response]() {
            it->second(response.object);
            this->handlers.erase(it);
            return G_SOURCE_REMOVE;
          });
        }
      }
    }
    is_running.store(false);
  }
  void start_loop() {
    if (is_running.load() || worker_thread.joinable())
      return;
    is_running = true;
    worker_thread = std::thread([this]() { this->loop(); });
    pthread_setname_np(worker_thread.native_handle(), "FlyingPaperTDLibWorker");
  }
  void stop_loop() {
    send(td::td_api::make_object<td::td_api::close>(), {});
    this->keep_running.store(false);
    if (worker_thread.joinable())
      worker_thread.join();
    client_manager.reset();
  }

  // TODO: Provide implementation to send request as json string.
  // virtual void send_json(const std::string &json) = 0;
};
} // namespace Telegram
} // namespace FlyingPaper

#endif // !FLYING_PAPER_CLIENT_MANAGER_IMPL_HPP
