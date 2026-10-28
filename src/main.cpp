#include "storage.hpp"
#include "ui.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
namespace {
long long number(const std::string& text,long long low,long long high) {
    std::size_t used=0;
    long long n=0;
    try { n=std::stoll(text,&used); } catch(...) { throw std::runtime_error("请输入有效整数"); }
    if(used!=text.size() || n<low || n>high) throw std::runtime_error("数字超出允许范围");
    return n;
}
void arity(const std::vector<std::string>& a,std::size_t low,std::size_t high) {
    if(a.size()<low || a.size()>high) throw std::runtime_error("参数数量不正确，输入 help 查看用法");
}
std::filesystem::path defaultSave() {
    if(const auto* x=std::getenv("XDG_DATA_HOME"); x && *x) return std::filesystem::path(x)/"arsenal/profile.sav";
    if(const auto* h=std::getenv("HOME"); h && *h) return std::filesystem::path(h)/".local/share/arsenal/profile.sav";
    return "profile.sav";
}
}
int main(int argc,char** argv) {
    using namespace arsenal;
    try {
        auto path=defaultSave();
        bool color=::isatty(STDOUT_FILENO)!=0 && !std::getenv("NO_COLOR");
        bool animate=color;
        std::uint32_t seed=std::random_device{}();
        for(int i=1;i<argc;++i) {
            const std::string arg=argv[i];
            if(arg=="--no-color") color=false;
            else if(arg=="--no-animation") animate=false;
            else if(arg=="--save" && i+1<argc) path=argv[++i];
            else if(arg=="--seed" && i+1<argc) seed=static_cast<std::uint32_t>(number(argv[++i],0,std::numeric_limits<std::uint32_t>::max()));
            else if(arg=="--help") {
                std::cout<<"arsenal [--save PATH] [--seed N] [--no-color] [--no-animation]\n";
                return 0;
            } else throw std::runtime_error("未知选项或缺少选项参数："+arg);
        }
        Storage storage(path);
        auto profile=storage.load(); // 解析失败立即退出，绝不以空档覆盖。
        Engine engine(seed);
        Engine lab(seed ^ 0x9e3779b9U); // 实验使用独立随机引擎。
        UI ui(color,animate);
        ui.banner();
        std::string line;
        while(std::cout<<"[积分 "<<profile.credits<<"] arsenal> " && std::getline(std::cin,line)) {
            std::istringstream in(line);
            std::vector<std::string> a;
            for(std::string token;in>>token;) a.push_back(token);
            if(a.empty()) continue;
            try {
                const auto& cmd=a[0];
                if(cmd=="quit" || cmd=="exit") { arity(a,1,1); break; }
                if(cmd=="help") { arity(a,1,1); ui.help(); }
                else if(cmd=="cases") { arity(a,1,1); ui.listCases(); }
                else if(cmd=="bag") {
                    arity(a,1,3);
                    const std::string sort=a.size()>1?a[1]:"id";
                    if(sort!="id" && sort!="price" && sort!="rarity") throw std::runtime_error("排序可选 id / price / rarity");
                    ui.inventory(profile,sort,a.size()>2?static_cast<int>(number(a[2],0,5)):-1);
                } else if(cmd=="stats") { arity(a,1,1); ui.statistics(profile); }
                else if(cmd=="achievements") {
                    arity(a,1,1);
                    const auto all=achievements(profile);
                    std::cout<<"已解锁 "<<all.size()<<" / 7\n";
                    for(const auto& s:all) std::cout<<" ★ "<<s<<'\n';
                } else if(cmd=="simulate") {
                    arity(a,3,3);
                    ui.simulate(lab,static_cast<int>(number(a[1],1,3))-1,static_cast<int>(number(a[2],1,1000000)));
                } else {
                    auto next=profile;
                    std::vector<Item> reveal;
                    std::string message;
                    if(cmd=="open") {
                        arity(a,2,3);
                        reveal=engine.open(next,static_cast<int>(number(a[1],1,3))-1,
                            a.size()>2?static_cast<int>(number(a[2],1,10)):1);
                    } else if(cmd=="sell") {
                        arity(a,2,2);
                        message="出售收入："+std::to_string(engine.sell(next,static_cast<std::uint64_t>(number(a[1],1,1000000000))))+" 积分";
                    } else if(cmd=="sell-low") {
                        arity(a,1,1); message="批量出售收入："+std::to_string(engine.sellLow(next))+" 积分";
                    } else if(cmd=="lock") {
                        arity(a,2,2);
                        message=engine.toggleLock(next,static_cast<std::uint64_t>(number(a[1],1,1000000000)))?"已锁定":"已解锁";
                    } else if(cmd=="daily") {
                        arity(a,1,1); message=engine.daily(next,today())?"补给到账：5000 积分":"今天已经领取补给";
                    } else if(cmd=="save") { arity(a,1,1); message="已保存"; }
                    else throw std::runtime_error("未知命令，输入 help 查看帮助");
                    storage.save(next); // 持久化成功后才提交内存状态与显示奖励。
                    profile=std::move(next);
                    if(!message.empty()) std::cout<<message<<'\n';
                    ui.reveal(reveal);
                }
            } catch(const std::exception& e) { std::cout<<"提示："<<e.what()<<'\n'; }
        }
        storage.save(profile);
        std::cout<<"存档已保存，下次见！\n";
    } catch(const std::exception& e) {
        std::cerr<<"错误："<<e.what()<<'\n';
        return 1;
    }
}
