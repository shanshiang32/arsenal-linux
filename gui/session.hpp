#pragma once
#include "engine.hpp"
#include "storage.hpp"
#include <QObject>
#include <QStringList>
#include <functional>
namespace arsenal {
// UI只通过Session修改档案；成功落盘后才广播changed。
class Session : public QObject {
    Q_OBJECT
    Storage storage_;
    Profile profile_;
    Engine engine_;
    QStringList journal_;
    bool transact(const QString& description, const std::function<void(Profile&)>& action);
public:
    explicit Session(const std::filesystem::path& path, QObject* parent=nullptr);
    const Profile& profile() const { return profile_; }
    const QStringList& journal() const { return journal_; }
    bool openCase(int index, int count, std::vector<Item>& result);
    bool sellItem(std::uint64_t id);
    bool lockItem(std::uint64_t id);
    bool sellLowItems();
    bool claimDaily();
    bool exportCsv(const QString& path);
    bool backup(const QString& path);
    void record(const QString& message);
signals:
    void changed();
    void failure(const QString& message);
};
}
