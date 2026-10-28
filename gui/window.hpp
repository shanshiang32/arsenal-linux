#pragma once
#include "pages.hpp"
#include <QMainWindow>
#include <QStackedWidget>
namespace arsenal {
class MainWindow : public QMainWindow {
    Q_OBJECT
    Session& session_;
    QStackedWidget* pages_;
    QLabel* balance_;
public:
    explicit MainWindow(Session& session);
    QStackedWidget* pages() const { return pages_; }
};
}
