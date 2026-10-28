#include "pages.hpp"
#include "reveal.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
namespace arsenal {
static QLabel* text(const QString& s,const char* role=nullptr) {
    auto* l=new QLabel(s);l->setWordWrap(true);if(role) l->setObjectName(role);return l;
}
StorePage::StorePage(Session& session,QWidget* parent):QWidget(parent),session_(session) {
    auto* box=new QVBoxLayout(this);box->setSpacing(20);box->setContentsMargins(30,28,30,28);
    box->addWidget(text("补给站 / SUPPLY DEPOT","eyebrow"));
    box->addWidget(text("下一件收藏，从这里开始。","pageTitle"));
    box->addWidget(text("挑选补给箱，探索 24 件原创武器涂装。所有交易仅使用虚拟积分。","muted"));
    summary_=text("");summary_->setObjectName("infoBox");box->addWidget(summary_);
    auto* cards=new QHBoxLayout;cards->setSpacing(18);
    for(std::size_t n=0;n<cases().size();++n) {
        const int index=static_cast<int>(n);const auto& c=cases()[n];
        auto* card=new QFrame;card->setObjectName("card");auto* v=new QVBoxLayout(card);v->setContentsMargins(22,18,22,24);v->setSpacing(12);
        v->addWidget(text(QString("COLLECTION  /  0%1").arg(n+1),"eyebrow"));
        auto* image=new QLabel;image->setPixmap(caseArt(index,QSize(280,175)));image->setAlignment(Qt::AlignCenter);v->addWidget(image);
        v->addWidget(text(QString::fromStdString(c.name),"sectionTitle"));
        v->addWidget(text(QString::number(c.cost)+" 积分 / 箱","price"));
        QString odds;
        for(std::size_t t=0;t<tiers;++t)
            odds+=QString("<span style='color:%1'>%2</span> %3%<br>")
                .arg(tierColor(static_cast<int>(t)).name(),QString::fromStdString(tierName(static_cast<int>(t))),QString::number(c.weights[t]/100.0,'f',2));
        v->addWidget(text(odds));
        auto* qty=new QComboBox;qty->addItem("单箱探索",1);qty->addItem("五箱连开",5);qty->addItem("十箱连开",10);
        v->addWidget(qty);
        auto* open=new QPushButton("开启补给箱");open->setObjectName("primary");open->setProperty("caseIndex",index);v->addWidget(open);
        connect(open,&QPushButton::clicked,this,[this,index,qty]{
            std::vector<Item> result;
            if(session_.openCase(index,qty->currentData().toInt(),result)) { RevealDialog dialog(std::move(result),this);dialog.exec(); }
        });
        cards->addWidget(card,1);
    }
    box->addLayout(cards);box->addWidget(text("独立随机 · 无保底机制 · 公开概率 · 开箱结果自动保存","muted"));box->addStretch();
    auto update=[this]{
        const auto& p=session_.profile();
        summary_->setText(QString("账户余额  %1 积分     /     已开启  %2 箱     /     背包收藏  %3 件")
            .arg(p.credits).arg(p.opened).arg(p.inventory.size()));
    };
    connect(&session_,&Session::changed,this,update);update();
}
}
