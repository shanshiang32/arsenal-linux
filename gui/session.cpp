#include "session.hpp"
#include <QDateTime>
#include <QSaveFile>
#include <QTextStream>
#include <QFileInfo>
namespace arsenal {
Session::Session(const std::filesystem::path& path,QObject* parent)
    : QObject(parent),storage_(path),profile_(storage_.load()) { record("已加载本地档案"); }
void Session::record(const QString& message) {
    journal_.prepend(QDateTime::currentDateTime().toString("HH:mm:ss  ")+message);
    while(journal_.size()>200) journal_.removeLast();
}
bool Session::transact(const QString& description,const std::function<void(Profile&)>& action) {
    try {
        auto next=profile_;
        action(next);
        storage_.save(next);
        profile_=std::move(next);
        record(description);
        emit changed();
        return true;
    } catch(const std::exception& e) { emit failure(QString::fromUtf8(e.what())); return false; }
}
bool Session::openCase(int index,int count,std::vector<Item>& result) {
    std::vector<Item> pending;
    const bool ok=transact(QString("开启 %1 箱").arg(count),[&](Profile& p){pending=engine_.open(p,index,count);});
    if(ok) result=std::move(pending);
    return ok;
}
bool Session::sellItem(std::uint64_t id) {
    return transact(QString("出售饰品 #%1").arg(id),[&](Profile& p){engine_.sell(p,id);});
}
bool Session::lockItem(std::uint64_t id) {
    return transact(QString("切换饰品 #%1 锁定状态").arg(id),[&](Profile& p){engine_.toggleLock(p,id);});
}
bool Session::sellLowItems() { return transact("批量出售低等级饰品",[&](Profile& p){engine_.sellLow(p);}); }
bool Session::claimDaily() {
    return transact("领取每日补给",[&](Profile& p){
        if(!engine_.daily(p,today())) throw std::runtime_error("今天已领取补给，请明天再来。");
    });
}
bool Session::exportCsv(const QString& path) {
    QSaveFile file(path);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text)) { emit failure(file.errorString()); return false; }
    QTextStream out(&file);
    out<<"ID,Weapon,Finish,Rarity,Wear,Pattern,StatTrak,Locked,Value\n";
    for(const auto& i:profile_.inventory) {
        const auto& skin=catalog().at(static_cast<std::size_t>(i.skin));
        // 本项目内置名称不含逗号、引号或换行；不导入不可信CSV字段。
        out<<i.id<<','<<QString::fromStdString(skin.weapon)<<','<<QString::fromStdString(skin.finish)
           <<','<<skin.tier<<','<<QString::number(i.wear,'f',8)<<','<<i.pattern
           <<','<<i.stattrak<<','<<i.locked<<','<<value(i)<<'\n';
    }
    out.flush();
    if(out.status()!=QTextStream::Ok || !file.commit()) { emit failure("导出CSV失败"); return false; }
    record("导出背包CSV"); emit changed(); return true;
}
bool Session::backup(const QString& path) {
    try {
        // 文件选择器禁止选到当前存档；另一个进程已使用的备份路径也会被锁保护。
        Storage target(path.toStdString());
        target.save(profile_);
        record("已备份档案"); emit changed(); return true;
    } catch(const std::exception& e) { emit failure(QString::fromUtf8(e.what())); return false; }
}
}
