#include "engine.hpp"
#include <algorithm>
#include <stdexcept>
namespace arsenal {
Item Engine::roll(int index) {
    const auto& c = cases().at(static_cast<std::size_t>(index));
    std::discrete_distribution<int> rarity(c.weights.begin(), c.weights.end());
    const int tier = rarity(rng_);
    std::vector<int> pool;
    for (std::size_t n=0; n<catalog().size(); ++n)
        if (catalog()[n].tier == tier) pool.push_back(static_cast<int>(n));
    std::uniform_int_distribution<std::size_t> choice(0, pool.size()-1);
    Item item;
    item.skin = pool[choice(rng_)];
    item.wear = std::uniform_real_distribution<double>(0, 1)(rng_);
    item.pattern = std::uniform_int_distribution<int>(0, 999)(rng_);
    item.stattrak = std::bernoulli_distribution(.1)(rng_);
    return item;
}
std::vector<Item> Engine::open(Profile& p, int index, int count) {
    if (index < 0 || static_cast<std::size_t>(index)>=cases().size()) throw std::runtime_error("箱子编号无效");
    if (count < 1 || count > 10) throw std::runtime_error("每次可开 1 至 10 箱");
    if (p.inventory.size()+static_cast<std::size_t>(count)>100000) throw std::runtime_error("背包已满");
    const auto cost = cases()[static_cast<std::size_t>(index)].cost * count;
    if (p.credits < cost) throw std::runtime_error("积分不足，可领取补给或出售饰品");
    auto next = p; // 在副本完成操作，异常时不留下半笔交易。
    std::vector<Item> items;
    for (int n=0; n<count; ++n) {
        auto item = roll(index);
        item.id = next.nextId++;
        next.inventory.push_back(item);
        ++next.drops[static_cast<std::size_t>(catalog()[static_cast<std::size_t>(item.skin)].tier)];
        items.push_back(item);
    }
    next.credits -= cost;
    next.spent += cost;
    next.opened += static_cast<std::uint64_t>(count);
    validate(next);
    p = std::move(next);
    return items;
}
std::int64_t Engine::sell(Profile& p, std::uint64_t id) {
    const auto it = std::find_if(p.inventory.begin(), p.inventory.end(), [id](const Item& i){return i.id==id;});
    if (it==p.inventory.end()) throw std::runtime_error("未找到该饰品 ID");
    if (it->locked) throw std::runtime_error("该饰品已锁定，请先解锁");
    const auto price = value(*it);
    p.credits += price;
    p.earned += price;
    p.inventory.erase(it);
    return price;
}
std::int64_t Engine::sellLow(Profile& p) {
    std::int64_t total = 0;
    auto it = p.inventory.begin();
    while (it != p.inventory.end()) {
        if (!it->locked && catalog()[static_cast<std::size_t>(it->skin)].tier <= 1) {
            total += value(*it);
            it = p.inventory.erase(it);
        } else ++it;
    }
    p.credits += total;
    p.earned += total;
    return total;
}
bool Engine::toggleLock(Profile& p, std::uint64_t id) {
    for (auto& item : p.inventory) if (item.id==id) return item.locked = !item.locked;
    throw std::runtime_error("未找到该饰品 ID");
}
bool Engine::daily(Profile& p, const std::string& date) {
    if (p.lastDaily >= date && p.lastDaily!="-") return false;
    p.credits += 5000;
    p.lastDaily = date;
    return true;
}
std::vector<std::string> achievements(const Profile& p) {
    std::vector<std::string> a;
    if (p.opened) a.push_back("初次出击：完成一次开箱");
    if (p.opened>=10) a.push_back("十连挑战：累计开启十箱");
    if (p.opened>=100) a.push_back("百箱老兵：累计开启一百箱");
    if (p.drops[4]) a.push_back("红色警报：获得隐秘级饰品");
    if (p.drops[5]) a.push_back("金色传说：获得传说级饰品");
    if (p.earned>=10000) a.push_back("交易达人：累计出售收入一万积分");
    if (p.inventory.size()>=20) a.push_back("收藏家：同时拥有二十件饰品");
    return a;
}
}
