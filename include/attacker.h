#include <string>
#include <vector>
#include <atomic>

void HTTPFire(const std::vector<std::string> &apiList, std::atomic<bool> &shouldStop);

void TCPFire(const std::vector<std::string> &apiList, std::atomic<bool> &shouldStop);
