#pragma once
#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

template <std::equality_comparable TClient>
class ClientManager {
public:
  void addClient(const TClient& client) {
    std::lock_guard guard(clientsMutex);
    clients.push_back(client);
  }
  void removeClient(const TClient& client) {
    std::lock_guard guard(clientsMutex);
    const auto removed =
        std::ranges::remove(clients.begin(), clients.end(), client);
    clients.erase(removed.begin(), removed.end());
  }
  void forEach(const std::function<void(const TClient&)> func) {
    std::lock_guard guard(clientsMutex);
    for (const auto& client : clients) {
      func(client);
    }
  }
private:
  std::vector<TClient> clients;
  std::mutex clientsMutex;
};
