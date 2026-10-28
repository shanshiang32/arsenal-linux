#pragma once
#include "session.hpp"
#include "art.hpp"
#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QSpinBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
namespace arsenal {
class StorePage : public QWidget {
    Session& session_;
    QLabel* summary_;
public:
    explicit StorePage(Session& session,QWidget* parent=nullptr);
};
class InventoryPage : public QWidget {
    Session& session_;
    QTableWidget* table_;
    QLineEdit* search_;
    QComboBox* rarity_;
    QLabel* count_;
    std::uint64_t selectedId() const;
public:
    explicit InventoryPage(Session& session,QWidget* parent=nullptr);
    void refresh();
};
class StatsPage : public QWidget {
    Session& session_;
    QLabel* summary_;
    DropChart* chart_;
    QTextEdit* achievements_;
    QTextEdit* journal_;
public:
    explicit StatsPage(Session& session,QWidget* parent=nullptr);
    void refresh();
};
class LabPage : public QWidget {
    Engine engine_;
    QTimer timer_;
    QComboBox* case_;
    QSpinBox* count_;
    QPushButton* start_;
    QPushButton* cancel_;
    QProgressBar* progress_;
    QLabel* result_;
    DropChart* chart_;
    int done_=0,target_=0,index_=0;
    std::int64_t total_=0;
    std::array<std::uint64_t,tiers> hits_{};
    void step();
public:
    explicit LabPage(QWidget* parent=nullptr);
};
QWidget* helpPage(QWidget* parent=nullptr);
}
