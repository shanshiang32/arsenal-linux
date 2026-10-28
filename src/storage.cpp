#include "storage.hpp"
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
namespace arsenal {
Storage::Storage(std::filesystem::path path) : path_(std::move(path)) {
    if (!path_.parent_path().empty()) std::filesystem::create_directories(path_.parent_path());
    const auto name = path_.string()+".lock";
    lock_ = ::open(name.c_str(), O_CREAT|O_RDWR, 0600);
    if (lock_ < 0) throw std::runtime_error("无法创建存档锁");
    if (flock(lock_, LOCK_EX|LOCK_NB) != 0) {
        ::close(lock_); lock_=-1;
        throw std::runtime_error("该存档正被另一个进程使用");
    }
}
Storage::~Storage() { if(lock_>=0) { flock(lock_, LOCK_UN); ::close(lock_); } }
Profile Storage::load() const {
    if (!std::filesystem::exists(path_)) return {};
    std::ifstream in(path_);
    if (!in) throw std::runtime_error("无法读取存档");
    std::string magic;
    int version = 0;
    Profile p;
    std::size_t count = 0;
    if (!(in>>magic>>version) || magic!="ARSENAL" || version!=1)
        throw std::runtime_error("存档格式或版本不支持，请保留原文件");
    if (!(in>>p.credits>>p.nextId>>p.opened>>p.spent>>p.earned>>p.lastDaily))
        throw std::runtime_error("存档头损坏");
    for(auto& n:p.drops) if(!(in>>n)) throw std::runtime_error("统计字段损坏");
    if (!(in>>count) || count>100000) throw std::runtime_error("背包大小无效");
    for(std::size_t n=0;n<count;++n) {
        Item i;
        int st=0, locked=0;
        if (!(in>>i.id>>i.skin>>i.wear>>i.pattern>>st>>locked) ||
            (st!=0 && st!=1) || (locked!=0 && locked!=1))
            throw std::runtime_error("饰品字段损坏");
        i.stattrak=st!=0; i.locked=locked!=0;
        p.inventory.push_back(i);
    }
    in>>std::ws;
    if (!in.eof()) throw std::runtime_error("存档含多余字段");
    validate(p);
    return p;
}
void Storage::save(const Profile& p) const {
    validate(p);
    const auto temp=path_.string()+".tmp";
    std::ofstream out(temp, std::ios::trunc);
    if (!out) throw std::runtime_error("无法写入临时存档");
    out<<"ARSENAL 1\n"<<p.credits<<' '<<p.nextId<<' '<<p.opened<<' '
       <<p.spent<<' '<<p.earned<<' '<<p.lastDaily<<'\n';
    for(auto n:p.drops) out<<n<<' ';
    out<<'\n'<<p.inventory.size()<<'\n'<<std::setprecision(17);
    for(const auto& i:p.inventory)
        out<<i.id<<' '<<i.skin<<' '<<i.wear<<' '<<i.pattern<<' '<<i.stattrak<<' '<<i.locked<<'\n';
    out.flush();
    if (!out) throw std::runtime_error("存档写入失败，旧存档已保留");
    out.close();
    if (!out) throw std::runtime_error("存档关闭失败，旧存档已保留");
    std::filesystem::rename(temp, path_); // 同目录 rename，避免读到半份存档。
}
}
