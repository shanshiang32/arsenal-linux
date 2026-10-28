#include "model.hpp"
#include <cmath>
#include <ctime>
#include <set>
#include <stdexcept>
namespace arsenal {
const std::vector<Skin>& catalog() {
    static const std::vector<Skin> data = {
        {"Glock", "沙丘", 0, 80}, {"P250", "都市迷彩", 0, 100},
        {"MP9", "深海", 0, 120}, {"Nova", "极地", 0, 90},
        {"UMP", "电路板", 1, 260}, {"MAC-10", "霓虹街区", 1, 300},
        {"FAMAS", "夜航", 1, 330}, {"Galil", "蓝色脉冲", 1, 280},
        {"M4A4", "机械之心", 2, 900}, {"AK-47", "赤色轨迹", 2, 1100},
        {"AWP", "冰河", 2, 1300}, {"USP", "星轨", 2, 950},
        {"AK-47", "量子裂隙", 3, 3800}, {"M4A1-S", "极光", 3, 4200},
        {"AWP", "星云", 3, 5000}, {"Desert Eagle", "金色余晖", 3, 3500},
        {"AK-47", "红莲", 4, 16000}, {"AWP", "龙息", 4, 20000},
        {"M4A4", "天启", 4, 18000}, {"USP", "白夜", 4, 14000},
        {"蝴蝶刀", "多普勒", 5, 90000}, {"爪子刀", "渐变之色", 5, 110000},
        {"刺刀", "星河", 5, 80000}, {"手套", "超导体", 5, 100000}
    };
    return data;
}
const std::vector<Case>& cases() {
    // 自定义教学概率，并非任何商业游戏的实际概率。权重均合计 10000。
    static const std::vector<Case> data = {
        {"边境补给箱", 500, {7000, 2200, 600, 160, 30, 10}},
        {"霓虹武器箱", 1200, {5000, 3200, 1300, 400, 80, 20}},
        {"星际典藏箱", 3000, {3000, 3500, 2400, 900, 150, 50}}
    };
    return data;
}
std::string tierName(int t) {
    static const std::array<std::string, tiers> names = {"消费级", "军规级", "受限级", "保密级", "隐秘级", "传说级"};
    return names.at(static_cast<std::size_t>(t));
}
std::string wearName(double w) {
    if (w < .07) return "崭新出厂";
    if (w < .15) return "略有磨损";
    if (w < .38) return "久经沙场";
    if (w < .45) return "破损不堪";
    return "战痕累累";
}
std::int64_t value(const Item& i) {
    const auto& s = catalog().at(static_cast<std::size_t>(i.skin));
    return static_cast<std::int64_t>(std::llround(static_cast<double>(s.base) * (1.5-i.wear) * (i.stattrak?1.8:1.0)));
}
std::string itemName(const Item& i) {
    const auto& s = catalog().at(static_cast<std::size_t>(i.skin));
    return (i.stattrak ? "StatTrak™ " : "") + s.weapon + " | " + s.finish;
}
std::string today() {
    const auto now = std::time(nullptr);
    std::tm result{};
    localtime_r(&now, &result);
    char out[16];
    std::strftime(out, sizeof(out), "%Y-%m-%d", &result);
    return out;
}
void validate(const Profile& p) {
    if (p.credits < 0 || p.credits > 1000000000000LL || p.spent < 0 || p.earned < 0 ||
        p.opened > 1000000000ULL || p.nextId == 0 || p.inventory.size() > 100000)
        throw std::runtime_error("存档数值越界");
    std::uint64_t total = 0;
    for (auto count : p.drops) {
        if (count > p.opened) throw std::runtime_error("掉落统计无效");
        total += count;
    }
    if (total != p.opened) throw std::runtime_error("掉落统计与开箱次数不一致");
    std::set<std::uint64_t> ids;
    for (const auto& i : p.inventory) {
        if (!i.id || i.id >= p.nextId || !ids.insert(i.id).second || i.skin < 0 ||
            static_cast<std::size_t>(i.skin) >= catalog().size() || !std::isfinite(i.wear) ||
            i.wear < 0 || i.wear > 1 || i.pattern < 0 || i.pattern > 999)
            throw std::runtime_error("背包数据无效");
    }
    if (p.lastDaily != "-" && (p.lastDaily.size()!=10 || p.lastDaily[4]!='-' || p.lastDaily[7]!='-'))
        throw std::runtime_error("补给日期无效");
}
}
