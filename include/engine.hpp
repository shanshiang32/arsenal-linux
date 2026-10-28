#pragma once
#include "model.hpp"
#include <random>
namespace arsenal {
class Engine {
    std::mt19937 rng_;
public:
    explicit Engine(std::uint32_t seed = std::random_device{}()) : rng_(seed) {}
    Item roll(int caseIndex);
    std::vector<Item> open(Profile& profile, int caseIndex, int count);
    std::int64_t sell(Profile& profile, std::uint64_t id);
    std::int64_t sellLow(Profile& profile);
    bool toggleLock(Profile& profile, std::uint64_t id);
    bool daily(Profile& profile, const std::string& date);
};
std::vector<std::string> achievements(const Profile& profile);
}
