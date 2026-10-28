#pragma once
#include "art.hpp"
#include <QDialog>
#include <QVariantAnimation>
namespace arsenal {
class Reel : public QWidget {
    std::vector<int> skins_;
    double offset_=0;
public:
    explicit Reel(int winner,QWidget* parent=nullptr);
    void setOffset(double offset) { offset_=offset;update(); }
protected:
    void paintEvent(QPaintEvent*) override;
};
class RevealDialog : public QDialog {
    Q_OBJECT
    std::vector<Item> items_;
    std::size_t current_=0;
    Reel* reel_=nullptr;
    void showNext();
public:
    RevealDialog(std::vector<Item> items,QWidget* parent=nullptr);
};
}
