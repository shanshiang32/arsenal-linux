#include "window.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QScrollArea>
namespace arsenal {
MainWindow::MainWindow(Session& s):session_(s) {
    setWindowTitle("ARSENAL · 军械库");resize(1320,870);setMinimumSize(1100,740);
    auto* root=new QWidget;setCentralWidget(root);auto* layout=new QHBoxLayout(root);layout->setContentsMargins(0,0,0,0);layout->setSpacing(0);
    auto* side=new QFrame;side->setObjectName("sidebar");side->setFixedWidth(212);auto* nav=new QVBoxLayout(side);nav->setContentsMargins(20,32,20,24);nav->setSpacing(12);
    auto* brand=new QLabel("ARSENAL");brand->setObjectName("brand");nav->addWidget(brand);
    auto* tag=new QLabel("军械库 / COLLECTION LAB");tag->setObjectName("eyebrow");nav->addWidget(tag);nav->addSpacing(32);
    const QStringList names={"01   补给站","02   我的军械库","03   数据与成就","04   概率实验室","05   关于与帮助"};
    auto* group=new QButtonGroup(this);group->setExclusive(true);
    pages_=new QStackedWidget;pages_->setObjectName("pages");
    for(int n=0;n<names.size();++n) {
        auto* button=new QPushButton(names[n]);button->setObjectName(QString("nav%1").arg(n));button->setProperty("nav",true);button->setCheckable(true);
        group->addButton(button,n);nav->addWidget(button);if(n==0) button->setChecked(true);
        connect(button,&QPushButton::clicked,this,[this,n]{pages_->setCurrentIndex(n);});
    }
    nav->addStretch();balance_=new QLabel;balance_->setObjectName("balance");nav->addWidget(balance_);
    auto* daily=new QPushButton("领取每日补给");daily->setObjectName("dailyButton");nav->addWidget(daily);
    connect(daily,&QPushButton::clicked,&session_,&Session::claimDaily);
    auto* backup=new QPushButton("备份存档");nav->addWidget(backup);
    connect(backup,&QPushButton::clicked,this,[this]{
        const auto path=QFileDialog::getSaveFileName(this,"备份档案","arsenal-backup.sav","存档 (*.sav)");
        if(!path.isEmpty()) session_.backup(path);
    });
    auto* foot=new QLabel("DESKTOP EDITION  2.0\n纯虚拟积分 · 自动保存");foot->setObjectName("muted");nav->addWidget(foot);
    auto* storeScroll=new QScrollArea;storeScroll->setWidgetResizable(true);storeScroll->setFrameShape(QFrame::NoFrame);storeScroll->setWidget(new StorePage(s));
    pages_->addWidget(storeScroll);pages_->addWidget(new InventoryPage(s));pages_->addWidget(new StatsPage(s));pages_->addWidget(new LabPage);pages_->addWidget(helpPage());
    layout->addWidget(side);layout->addWidget(pages_,1);
    auto update=[this]{balance_->setText(QString("可用积分\n%1").arg(session_.profile().credits));statusBar()->showMessage("档案已加载 · 每笔操作自动保存");};
    connect(&s,&Session::changed,this,update);
    connect(&s,&Session::failure,this,[this](const QString& error){QMessageBox::warning(this,"操作未完成",error);});update();
}
}
