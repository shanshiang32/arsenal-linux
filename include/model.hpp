#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace arsenal {
constexpr std::size_t tiers = 6;
struct Skin { std::string weapon, finish; int tier; std::int64_t base; };
struct Case { std::string name; std::int64_t cost; std::array<int, tiers> weights; };
struct Item {
    std::uint64_t id = 0;
    int skin = 0;
    double wear = 0;
    int pattern = 0;
    bool stattrak = false;
    bool locked = false;
};
struct Profile {
    std::int64_t credits = 10000;
    std::uint64_t nextId = 1, opened = 0;
    std::int64_t spent = 0, earned = 0;
    std::array<std::uint64_t, tiers> drops{};
    std::string lastDaily = "-";
    std::vector<Item> inventory;
};
const std::vector<Skin>& catalog();
const std::vector<Case>& cases();
std::string tierName(int tier);
std::string wearName(double wear);
std::int64_t value(const Item& item);
std::string itemName(const Item& item);
std::string today();
void validate(const Profile& profile);
}
