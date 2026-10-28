#include "art.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <algorithm>
#include <numeric>
namespace arsenal {
QColor tierColor(int t) {
    static const QColor palette[]={QColor("#93a5b9"),QColor("#5898fa"),QColor("#9170ff"),
        QColor("#dc72ec"),QColor("#ff6879"),QColor("#efc66a")};
    return palette[std::clamp(t,0,5)];
}
QPixmap itemArt(int skinIndex,QSize size) {
    QPixmap pix(size); pix.fill(Qt::transparent);
    QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing);
    p.scale(size.width()/320.0,size.height()/170.0);
    const auto& skin=catalog().at(static_cast<std::size_t>(skinIndex));
    const auto color=tierColor(skin.tier);
    QRadialGradient glow(160,90,145);glow.setColorAt(0,QColor(color.red(),color.green(),color.blue(),55));
    glow.setColorAt(1,QColor(15,22,34,0));p.fillRect(QRectF(0,0,320,170),glow);
    p.translate(160,85);p.rotate(-14);p.translate(-160,-85);
    QPainterPath shape;
    if(skin.tier==5) {
        shape.moveTo(45,99);shape.lineTo(105,83);shape.lineTo(250,54);
        shape.lineTo(218,90);shape.lineTo(115,101);shape.lineTo(56,119);shape.closeSubpath();
    } else {
        // 原创抽象武器剪影，不使用商业游戏贴图。
        shape.moveTo(30,80);shape.lineTo(74,66);shape.lineTo(88,76);shape.lineTo(177,76);
        shape.lineTo(182,67);shape.lineTo(230,68);shape.lineTo(230,77);shape.lineTo(283,77);
        shape.lineTo(283,86);shape.lineTo(221,87);shape.lineTo(206,96);shape.lineTo(167,96);
        shape.lineTo(158,128);shape.lineTo(140,125);shape.lineTo(143,97);
        shape.lineTo(124,97);shape.lineTo(117,120);shape.lineTo(103,116);shape.lineTo(109,94);
        shape.lineTo(82,91);shape.lineTo(39,106);shape.closeSubpath();
    }
    QLinearGradient paint(50,65,245,110);paint.setColorAt(0,color.darker(170));
    paint.setColorAt(.45,color.lighter(140));paint.setColorAt(1,color.darker(120));
    p.setPen(QPen(color.lighter(150),1.3));p.setBrush(paint);p.drawPath(shape);
    p.save();p.setClipPath(shape);p.setPen(QPen(QColor(255,255,255,75),2));
    for(int x=20;x<310;x+=20) p.drawLine(x,40,x-55,140);
    p.restore();
    return pix;
}
QPixmap caseArt(int index,QSize size) {
    QPixmap pix(size);pix.fill(Qt::transparent);QPainter p(&pix);p.setRenderHint(QPainter::Antialiasing);
    p.scale(size.width()/320.0,size.height()/190.0);
    const QColor accent=index==0?QColor("#789dba"):index==1?QColor("#a884ff"):QColor("#efc66a");
    QRadialGradient glow(160,100,150);glow.setColorAt(0,QColor(accent.red(),accent.green(),accent.blue(),50));
    glow.setColorAt(1,QColor(15,22,34,0));p.fillRect(QRectF(0,0,320,190),glow);
    p.setPen(QPen(accent.darker(160),3));p.setBrush(QColor("#192634"));
    p.drawRoundedRect(QRectF(64,53,192,108),9,9);
    p.setBrush(QColor("#293c4d"));p.drawRoundedRect(QRectF(127,37,66,24),5,5);
    QLinearGradient g(0,60,0,155);g.setColorAt(0,accent.lighter(110));g.setColorAt(1,accent.darker(190));
    p.setBrush(g);p.setPen(QPen(accent,2));p.drawRoundedRect(QRectF(56,62,207,91),8,8);
    p.setPen(QPen(QColor("#111b28"),6));p.drawLine(85,64,85,150);p.drawLine(234,64,234,150);
    p.setPen(accent.lighter(160));p.setBrush(QColor("#13202d"));p.drawRoundedRect(QRectF(135,88,49,35),5,5);
    p.setFont(QFont("Sans",16,QFont::Bold));p.drawText(QRectF(135,88,49,35),Qt::AlignCenter,"A");
    p.setPen(QPen(accent.lighter(140),2));p.drawLine(105,140,216,140);
    return pix;
}
DropChart::DropChart(QWidget* parent):QWidget(parent) { setMinimumHeight(260); }
void DropChart::setData(std::array<std::uint64_t,tiers> h,std::array<int,tiers> w) {
    hits_=h;weights_=w;update();
}
void DropChart::paintEvent(QPaintEvent*) {
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
    const auto total=std::accumulate(hits_.begin(),hits_.end(),std::uint64_t(0));
    const double col=width()/6.0;
    const double base=height()-55;
    p.setFont(QFont("Noto Sans CJK SC",10));
    for(std::size_t n=0;n<tiers;++n) {
        const double ratio=total?static_cast<double>(hits_[n])/static_cast<double>(total):0;
        const double x=static_cast<double>(n)*col+col*.28;
        const double h=ratio*(height()-100);
        p.setBrush(QColor("#202d3e"));p.setPen(Qt::NoPen);p.drawRoundedRect(QRectF(x,40,col*.44,base-40),5,5);
        p.setBrush(tierColor(static_cast<int>(n)));p.drawRoundedRect(QRectF(x,base-h,col*.44,h),5,5);
        p.setPen(QColor("#dce7f5"));
        p.drawText(QRectF(static_cast<double>(n)*col,base+10,col,25),Qt::AlignCenter,QString::fromStdString(tierName(static_cast<int>(n))));
        p.drawText(QRectF(static_cast<double>(n)*col,10,col,25),Qt::AlignCenter,QString::number(ratio*100,'f',2)+"%");
        if(weights_[n]) {
            const double y=base-(weights_[n]/10000.0)*(height()-100);
            p.setPen(QPen(QColor("#ffffff"),2,Qt::DashLine));p.drawLine(QPointF(x-6,y),QPointF(x+col*.44+6,y));
        }
    }
}
}
