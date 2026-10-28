#pragma once
#include "model.hpp"
#include <QColor>
#include <QPixmap>
#include <QWidget>
namespace arsenal {
QColor tierColor(int tier);
QPixmap itemArt(int skin, QSize size);
QPixmap caseArt(int index, QSize size);
class DropChart : public QWidget {
    std::array<std::uint64_t,tiers> hits_{};
    std::array<int,tiers> weights_{};
public:
    explicit DropChart(QWidget* parent=nullptr);
    void setData(std::array<std::uint64_t,tiers> hits,std::array<int,tiers> weights={});
protected:
    void paintEvent(QPaintEvent*) override;
};
}
