#pragma once
#include "model.hpp"
#include <filesystem>
namespace arsenal {
class Storage {
    std::filesystem::path path_;
    int lock_ = -1;
public:
    explicit Storage(std::filesystem::path path);
    ~Storage();
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;
    Profile load() const;
    void save(const Profile& profile) const;
};
}
