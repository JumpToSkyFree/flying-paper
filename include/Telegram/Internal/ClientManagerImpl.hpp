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
  std::unique_ptr<td::ClientManager> client_manager;

  std::atomic_bool is_running{false};
  std::atomic_bool keep_running{true};

  std::int32_t client_id{0};
  std::uint64_t query_id_counter{1};
  std::uint64_t current_query_id{0};
  std::uint64_t authentication_query_id{0};

  std::mutex handlers_mtx;
  std::mutex subscribers_mtx;

  std::thread worker_thread;

  std::uint64_t next_subscription_id{1};
  std::unordered_map<std::int32_t, std::unordered_map<std::uint64_t, Callback>>
      subscribers;
  std::unordered_map<std::uint64_t, Callback> handlers;

  ClientManager::ClientManagerAuthorizationParams params;

  Callback handle_authorization_error;
  std::function<void(std::int32_t)> handle_authentication;

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
    std::uint64_t subscribtion_id = ++this->next_subscription_id;
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
      td::ClientManager::Response response =
          this->client_manager->receive(10.0);
      if (!response.object)
        continue;

      auto shared_obj =
          std::shared_ptr<td::td_api::Object>(response.object.release());

      if (response.request_id == 0) {
        const std::int32_t type_id = shared_obj->get_id();

        std::lock_guard<std::mutex> lock(subscribers_mtx);
        auto it_map = this->subscribers.find(type_id);

        if (it_map != this->subscribers.end()) {
          for (auto const &[id, cb] : it_map->second) {
            // cb(response.object);
            peel::GLib::idle_add([cb, obj_copy = shared_obj]() {
              cb(obj_copy);
              return G_SOURCE_REMOVE;
            });
          }
        }
      } else {
        std::unordered_map<std::uint64_t, Callback>::iterator it =
            handlers.find(response.request_id);
        std::lock_guard<std::mutex> lock(handlers_mtx);
        if (it != handlers.end()) {
          peel::GLib::idle_add([it, obj_copy = shared_obj]() {
            it->second(obj_copy);
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
  void update_authorization_state(const SharedObject &obj) {
    const auto &update =
        ClientManager::cast<td::td_api::updateAuthorizationState>(obj);
    if (update.authorization_state_) {
      std::int32_t id = update.authorization_state_->get_id();
      if (id == td::td_api::authorizationStateWaitTdlibParameters::ID) {
        on_authorization_state_update();
      } else {
        if (this->handle_authentication)
          this->handle_authentication(id);
      }
    }
  }
  void authorize() {
    this->subscribe(
        td::td_api::updateAuthorizationState::ID,
        [this](const SharedObject &obj) { update_authorization_state(obj); });
  }
  void on_authorization_state_update() {
    ++authentication_query_id;
    auto request = td::make_tl_object<td::td_api::setTdlibParameters>();
    request->database_directory_ = "tdlib";
    request->use_message_database_ = true;
    request->use_secret_chats_ = true;
    request->api_id_ = this->params.api_id;
    request->api_hash_ = this->params.api_hash;
    request->system_language_code_ = this->params.system_language;
    request->device_model_ = "Desktop";
    request->application_version_ = "1.0";
    request->files_directory_ = "files";
    request->use_chat_info_database_ = true;
    this->send(std::move(request), [this](const SharedObject &obj) {
      this->authentication_query_handler(authentication_query_id, obj);
    });
  }
  void authentication_query_handler(std::uint64_t id, const SharedObject &obj) {
    if (id == authentication_query_id) {
      if (this->handle_authorization_error)
        this->handle_authorization_error(obj);
    }
  }
};
} // namespace Telegram
} // namespace FlyingPaper

#endif // !FLYING_PAPER_CLIENT_MANAGER_IMPL_HPP
