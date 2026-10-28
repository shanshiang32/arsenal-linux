#include "reveal.hpp"
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
namespace arsenal {
Reel::Reel(int winner,QWidget* parent):QWidget(parent) {
    setMinimumSize(700,235);
    for(int n=0;n<32;++n) skins_.push_back(QRandomGenerator::global()->bounded(static_cast<int>(catalog().size())));
    skins_[26]=winner;
}
void Reel::paintEvent(QPaintEvent*) {
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),QColor("#0c1420"));
    constexpr int cw=164;
    for(std::size_t n=0;n<skins_.size();++n) {
        const double x=width()/2.0-cw/2.0+static_cast<double>(n)*cw-offset_;
        if(x>width() || x+cw<0) continue;
        const auto& skin=catalog()[static_cast<std::size_t>(skins_[n])];
        p.setPen(Qt::NoPen);p.setBrush(QColor("#192536"));p.drawRoundedRect(QRectF(x+4,28,cw-8,175),8,8);
        p.drawPixmap(QRectF(x+4,35,cw-8,100),itemArt(skins_[n],QSize(312,200)),QRectF(0,0,312,200));
        p.setPen(QColor("#e1e9f5"));p.drawText(QRectF(x,143,cw,25),Qt::AlignCenter,QString::fromStdString(skin.weapon));
        p.setPen(tierColor(skin.tier));p.drawText(QRectF(x,168,cw,22),Qt::AlignCenter,QString::fromStdString(skin.finish));
        p.fillRect(QRectF(x+8,198,cw-16,3),tierColor(skin.tier));
    }
    p.setPen(QPen(QColor("#efc66a"),3));p.drawLine(width()/2,16,width()/2,height()-16);
    QPolygon arrow;arrow<<QPoint(width()/2-8,7)<<QPoint(width()/2+8,7)<<QPoint(width()/2,19);
    p.setBrush(QColor("#efc66a"));p.drawPolygon(arrow);
}
RevealDialog::RevealDialog(std::vector<Item> items,QWidget* parent):QDialog(parent),items_(std::move(items)) {
    setWindowTitle("开启补给箱");setModal(true);resize(800,450);
    showNext();
}
void RevealDialog::showNext() {
    if(layout()) { QLayoutItem* child;while((child=layout()->takeAt(0))) { if(child->widget()) child->widget()->deleteLater();delete child; } delete layout(); }
    auto* box=new QVBoxLayout(this);box->setContentsMargins(28,24,28,24);box->setSpacing(16);
    auto* title=new QLabel(QString("开箱揭晓   %1 / %2").arg(current_+1).arg(items_.size()));title->setObjectName("sectionTitle");box->addWidget(title);
    const Item item=items_.at(current_);
    reel_=new Reel(item.skin);box->addWidget(reel_);
    auto* detail=new QLabel("正在开启 · 奖励已安全存入背包");detail->setAlignment(Qt::AlignCenter);box->addWidget(detail);
    auto* next=new QPushButton(current_+1<items_.size()?"下一件":"收入背包");next->setObjectName("primary");next->setEnabled(false);box->addWidget(next);
    connect(next,&QPushButton::clicked,this,[this]{ if(++current_>=items_.size()) accept();else showNext(); });
    auto* anim=new QVariantAnimation(this);anim->setDuration(2200);anim->setStartValue(0.0);anim->setEndValue(26.0*164);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim,&QVariantAnimation::valueChanged,reel_,[this](const QVariant& v){reel_->setOffset(v.toDouble());});
    connect(anim,&QVariantAnimation::finished,detail,[detail,next,item]{
        detail->setText(QString::fromStdString(itemName(item))+"  ·  "+QString::fromStdString(wearName(item.wear))+"  ·  "+QString::number(value(item))+" 积分");
        next->setEnabled(true);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
}
