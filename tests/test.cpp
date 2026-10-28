#include "engine.hpp"
#include "storage.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
using namespace arsenal;
int checks=0;
void check(bool ok,const char* message) {
    ++checks;
    if(!ok) throw std::runtime_error(message);
}
template<class F> void rejects(F action,const char* message) {
    bool thrown=false;
    try { action(); } catch(const std::exception&) { thrown=true; }
    check(thrown,message);
}
void testModel() {
    check(catalog().size()==24,"catalog size");
    for(const auto& c:cases()) {
        int sum=0; for(auto n:c.weights) sum+=n;
        check(sum==10000,"probability sum");
    }
    check(wearName(.069)=="崭新出厂","wear boundary below");
    check(wearName(.07)=="略有磨损","wear boundary equal");
    Item i; i.skin=0; i.wear=.5;
    check(value(i)==80,"base valuation");
    i.stattrak=true; check(value(i)==144,"stattrak valuation");
    Profile p; validate(p);
    p.credits=-1; rejects([&]{validate(p);},"negative credits");
}
void testTransactions() {
    Engine e(42); Profile p;
    const auto items=e.open(p,0,10);
    check(p.credits==5000 && p.opened==10 && items.size()==10,"open accounting");
    check(p.inventory[9].id==10 && p.nextId==11,"unique IDs");
    rejects([&]{e.open(p,2,10);},"insufficient funds");
    check(p.credits==5000 && p.opened==10,"failed open unchanged");
    rejects([&]{e.open(p,-1,1);},"negative case");
    rejects([&]{e.open(p,0,0);},"zero count");
    check(e.toggleLock(p,1),"lock on");
    rejects([&]{e.sell(p,1);},"locked sale");
    e.sellLow(p);
    check(p.inventory.front().id==1,"batch retains locked item");
    check(!e.toggleLock(p,1),"lock off");
    const auto price=value(p.inventory.front());
    const auto before=p.credits;
    check(e.sell(p,1)==price && p.credits==before+price,"single sale");
    rejects([&]{e.sell(p,1);},"double sale");
    check(e.daily(p,"2026-10-28"),"first daily");
    check(!e.daily(p,"2026-10-28"),"duplicate daily");
    check(!e.daily(p,"2026-10-27"),"clock rollback");
    check(e.daily(p,"2026-10-29"),"next daily");
    validate(p);
    check(!achievements(p).empty(),"achievements");
}
void testRandom() {
    Engine a(123), b(123);
    std::array<int,tiers> hits{};
    constexpr int samples=100000;
    for(int n=0;n<samples;++n) {
        const auto i=a.roll(0), j=b.roll(0);
        if(i.skin!=j.skin || i.wear!=j.wear || i.pattern!=j.pattern || i.stattrak!=j.stattrak)
            throw std::runtime_error("seed reproducibility");
        ++hits[static_cast<std::size_t>(catalog()[static_cast<std::size_t>(i.skin)].tier)];
    }
    ++checks;
    for(std::size_t t=0;t<tiers;++t) {
        const double probability=cases()[0].weights[t]/10000.0;
        const double expected=samples*probability;
        const double deviation=std::sqrt(samples*probability*(1-probability));
        check(std::abs(hits[t]-expected)<7*deviation+5,"probability deviation");
    }
}
void testStorage(const std::filesystem::path& dir) {
    const auto path=dir/"profile.sav";
    Storage s(path); Profile p; Engine e(5);
    e.open(p,1,3); e.toggleLock(p,1); s.save(p);
    const auto restored=s.load();
    check(restored.credits==p.credits && restored.opened==3,"storage header roundtrip");
    check(restored.inventory[0].wear==p.inventory[0].wear && restored.inventory[0].locked,"item roundtrip");
    rejects([&]{Storage other(path);},"exclusive lock");
    auto invalid=p; invalid.inventory[1].id=1;
    rejects([&]{s.save(invalid);},"duplicate ID rejected");
    check(s.load().opened==3,"invalid save preserves old file");
    { std::ofstream out(path); out<<"ARSENAL 1\ninvalid\n"; }
    rejects([&]{s.load();},"corrupt save rejected");
}
int main() {
    auto dir=std::filesystem::temp_directory_path()/("arsenal-test-"+std::to_string(getpid()));
    try {
        std::filesystem::create_directory(dir);
        testModel(); testTransactions(); testRandom(); testStorage(dir);
        std::filesystem::remove_all(dir);
        std::cout<<"PASS: "<<checks<<" checks (including 100000 deterministic draws)\n";
    } catch(const std::exception& e) {
        std::filesystem::remove_all(dir);
        std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1;
    }
}
