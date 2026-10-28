#pragma once
#include "engine.hpp"
namespace arsenal {
class UI {
    bool color_, animate_;
public:
    UI(bool color, bool animate) : color_(color), animate_(animate) {}
    void banner() const;
    void help() const;
    void listCases() const;
    void showItem(const Item& item) const;
    void reveal(const std::vector<Item>& items) const;
    void inventory(const Profile& profile, const std::string& sort, int tier) const;
    void statistics(const Profile& profile) const;
    void simulate(Engine& engine, int caseIndex, int count) const;
};
}
