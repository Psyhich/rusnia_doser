#include <functional>
#include <string>
#include <vector>
#include <atomic>

using AttackerFunc = std::function<void(const std::vector<std::string> &, std::atomic<bool> &)>;

void HTTPFire(const std::vector<std::string> &apiList, std::atomic<bool> &shouldStop) noexcept;

void TCPFire(const std::vector<std::string> &apiList, std::atomic<bool> &shouldStop) noexcept;
