#include "ui.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
namespace arsenal {
void UI::banner() const {
    std::cout<<"\n╔══════════════════════════════════════════════╗\n"
               "║       A R S E N A L   军 械 库  v1.0         ║\n"
               "╚══════════════════════════════════════════════╝\n"
               "  Linux / C++17 / Make / Git\n"
               "  纯虚拟积分开箱 · 输入 help 查看命令\n\n";
}
void UI::help() const {
    std::cout<<"命令                         功能\n"
      "cases                        箱子 / 价格 / 公开概率\n"
      "open 箱子编号 [数量]         开箱（1~10箱）\n"
      "bag [id|price|rarity] [0~5]   背包排序 / 按稀有度筛选\n"
      "sell 饰品ID                  出售单件饰品\n"
      "sell-low                     批量出售未锁定的消费级/军规级\n"
      "lock 饰品ID                  切换锁定，避免误卖\n"
      "daily                        每日本地日期领取5000积分\n"
      "stats                        开箱统计、资产、收益\n"
      "achievements                 查看已解锁成就\n"
      "simulate 箱子编号 次数       概率实验（不消耗积分，不进背包）\n"
      "save                         保存\n"
      "help                         帮助\n"
      "quit                         保存并退出（Ctrl+D也可）\n";
}
void UI::listCases() const {
    for(std::size_t n=0;n<cases().size();++n) {
        const auto& c=cases()[n];
        std::cout<<'['<<n+1<<"] "<<c.name<<" / "<<c.cost<<" 积分\n";
        for(std::size_t t=0;t<tiers;++t)
            std::cout<<"  "<<tierName(static_cast<int>(t))<<' '
                     <<std::fixed<<std::setprecision(2)<<c.weights[t]/100.0<<"%";
        std::cout<<'\n';
    }
    std::cout<<"概率为本项目自定义；每次独立随机，没有保底。\n";
}
void UI::showItem(const Item& i) const {
    const auto t=catalog()[static_cast<std::size_t>(i.skin)].tier;
    static const char* colors[]={"\033[37m","\033[34m","\033[35m","\033[95m","\033[91m","\033[93m"};
    if(color_) std::cout<<colors[t];
    std::cout<<'#'<<i.id<<" ["<<tierName(t)<<"] "<<itemName(i);
    if(color_) std::cout<<"\033[0m";
    std::cout<<"\n  "<<wearName(i.wear)<<" / 磨损 "<<std::fixed<<std::setprecision(6)
             <<i.wear<<" / 模板 "<<i.pattern<<" / 估值 "<<value(i)<<" 积分"
             <<(i.locked?" / 已锁定":"")<<'\n';
}
void UI::reveal(const std::vector<Item>& items) const {
    for(const auto& item:items) {
        if(animate_) {
            std::cout<<"  [ 开启中 ";
            for(int n=0;n<12;++n) { std::cout<<"▰"<<std::flush; std::this_thread::sleep_for(std::chrono::milliseconds(45)); }
            std::cout<<" ]\n";
        }
        showItem(item);
    }
}
void UI::inventory(const Profile& p,const std::string& sort,int tier) const {
    auto bag=p.inventory;
    if(tier>=0) bag.erase(std::remove_if(bag.begin(),bag.end(),[tier](const Item& i){
        return catalog()[static_cast<std::size_t>(i.skin)].tier!=tier;
    }),bag.end());
    std::sort(bag.begin(),bag.end(),[&sort](const Item& a,const Item& b){
        if(sort=="price" && value(a)!=value(b)) return value(a)>value(b);
        const auto ta=catalog()[static_cast<std::size_t>(a.skin)].tier;
        const auto tb=catalog()[static_cast<std::size_t>(b.skin)].tier;
        if(sort=="rarity" && ta!=tb) return ta>tb;
        return a.id<b.id;
    });
    std::cout<<"背包共 "<<p.inventory.size()<<" 件，本次显示 "<<bag.size()<<" 件\n";
    for(const auto& i:bag) showItem(i);
}
void UI::statistics(const Profile& p) const {
    std::int64_t asset=0;
    for(const auto& i:p.inventory) asset+=value(i);
    std::cout<<"余额 "<<p.credits<<" | 背包估值 "<<asset<<" | 开箱 "<<p.opened<<" 次\n"
             <<"开箱支出 "<<p.spent<<" | 出售收入 "<<p.earned
             <<" | 饰品收益（含持仓） "<<p.earned+asset-p.spent<<'\n';
    for(std::size_t t=0;t<tiers;++t) {
        const double ratio=p.opened?100.0*static_cast<double>(p.drops[t])/static_cast<double>(p.opened):0;
        std::cout<<tierName(static_cast<int>(t))<<"  "<<p.drops[t]<<"  "
                 <<std::fixed<<std::setprecision(2)<<ratio<<"%\n";
    }
}
void UI::simulate(Engine& engine,int index,int count) const {
    std::array<int,tiers> hits{};
    std::int64_t values=0;
    for(int n=0;n<count;++n) {
        const auto i=engine.roll(index);
        ++hits[static_cast<std::size_t>(catalog()[static_cast<std::size_t>(i.skin)].tier)];
        values+=value(i);
    }
    std::cout<<"独立实验 "<<count<<" 次；理论概率 / 实测概率\n";
    for(std::size_t t=0;t<tiers;++t) {
        const double ratio=100.0*hits[t]/count;
        std::cout<<tierName(static_cast<int>(t))<<"  "<<std::fixed<<std::setprecision(2)
                 <<cases()[static_cast<std::size_t>(index)].weights[t]/100.0<<"% / "<<ratio<<"% "
                 <<std::string(static_cast<std::size_t>(ratio/2),'#')<<'\n';
    }
    std::cout<<"平均饰品估值 "<<values/count<<" / 单箱成本 "<<cases()[static_cast<std::size_t>(index)].cost<<'\n';
}
}
