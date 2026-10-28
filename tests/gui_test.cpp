#include "window.hpp"
#include "reveal.hpp"
#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QMessageBox>
#include <QDir>
using namespace arsenal;
class GuiTests : public QObject {
    Q_OBJECT
private slots:
    void sessionTransactions() {
        QTemporaryDir dir;QVERIFY(dir.isValid());
        const auto path=dir.filePath("profile.sav");
        {
            Session s(path.toStdString());QSignalSpy changed(&s,&Session::changed);QSignalSpy failed(&s,&Session::failure);
            std::vector<Item> result;
            QVERIFY(s.openCase(0,3,result));QCOMPARE(result.size(),std::size_t(3));QCOMPARE(s.profile().credits,std::int64_t(8500));QCOMPARE(changed.count(),1);
            QVERIFY(s.lockItem(result[0].id));const auto balance=s.profile().credits;
            QVERIFY(!s.sellItem(result[0].id));QCOMPARE(failed.count(),1);QCOMPARE(s.profile().credits,balance);
            QVERIFY(s.lockItem(result[0].id));QVERIFY(s.sellItem(result[0].id));QCOMPARE(s.profile().inventory.size(),std::size_t(2));
            QVERIFY(s.claimDaily());QVERIFY(!s.claimDaily());
            QVERIFY(s.exportCsv(dir.filePath("items.csv")));QFile csv(dir.filePath("items.csv"));QVERIFY(csv.open(QIODevice::ReadOnly));QVERIFY(csv.readAll().startsWith("ID,Weapon"));
            QVERIFY(s.backup(dir.filePath("backup.sav")));Storage backup(dir.filePath("backup.sav").toStdString());QCOMPARE(backup.load().opened,s.profile().opened);
        }
        Session reopened(path.toStdString());QCOMPARE(reopened.profile().opened,std::uint64_t(3));QCOMPARE(reopened.profile().inventory.size(),std::size_t(2));
    }
    void failedSaveDoesNotCommit() {
        QTemporaryDir dir;const auto path=dir.filePath("profile.sav");Session s(path.toStdString());
        QDir().mkdir(path+".tmp"); // 模拟写入目标不可用，检查内存没有提前提交。
        QSignalSpy failed(&s,&Session::failure);QSignalSpy changed(&s,&Session::changed);
        std::vector<Item> result;QVERIFY(!s.openCase(0,1,result));QCOMPARE(failed.count(),1);QCOMPARE(changed.count(),0);
        QCOMPARE(s.profile().credits,std::int64_t(10000));QVERIFY(result.empty());
    }
    void multiReveal() {
        Item a; a.id=1; a.skin=8; a.wear=.12;
        Item b; b.id=2; b.skin=20; b.wear=.03;
        RevealDialog dialog({a,b});dialog.show();
        auto buttons=dialog.findChildren<QPushButton*>();QVERIFY(!buttons.empty());
        auto* first=buttons.front();QVERIFY(!first->isEnabled());
        QTRY_VERIFY_WITH_TIMEOUT(first->isEnabled(),3500);QTest::mouseClick(first,Qt::LeftButton);
        QTest::qWait(40);auto next=dialog.findChildren<QPushButton*>();QVERIFY(!next.empty());
        auto* second=next.back();QTRY_VERIFY_WITH_TIMEOUT(second->isEnabled(),3500);
        QTest::mouseClick(second,Qt::LeftButton);QCOMPARE(dialog.result(),int(QDialog::Accepted));
    }
    void desktopInteraction() {
        QTemporaryDir dir;Session s(dir.filePath("profile.sav").toStdString());MainWindow window(s);window.show();QTest::qWait(100);
        auto* nav=window.findChild<QPushButton*>("nav1");QVERIFY(nav);QTest::mouseClick(nav,Qt::LeftButton);QCOMPARE(window.pages()->currentIndex(),1);
        std::vector<Item> items;QVERIFY(s.openCase(0,5,items));
        auto* table=window.findChild<QTableWidget*>("inventoryTable");QVERIFY(table);QCOMPARE(table->rowCount(),5);
        auto* search=window.findChild<QLineEdit*>("inventorySearch");search->setText("no-such-weapon");QCOMPARE(table->rowCount(),0);search->clear();QCOMPARE(table->rowCount(),5);
        table->selectRow(0);const auto id=table->item(0,0)->data(Qt::UserRole).toULongLong();
        QTest::mouseClick(window.findChild<QPushButton*>("lockSelected"),Qt::LeftButton);
        bool locked=false;for(const auto& i:s.profile().inventory) if(i.id==id) locked=i.locked;QVERIFY(locked);
        auto* daily=window.findChild<QPushButton*>("dailyButton");const auto balance=s.profile().credits;QTest::mouseClick(daily,Qt::LeftButton);QCOMPARE(s.profile().credits,balance+5000);
        // 验证真实开箱按钮可打开动画，并且关掉动画窗口后奖励仍在。
        QTest::mouseClick(window.findChild<QPushButton*>("nav0"),Qt::LeftButton);
        QPushButton* open=nullptr;for(auto* b:window.findChildren<QPushButton*>()) if(b->property("caseIndex").isValid()) {open=b;break;}
        QVERIFY(open);const auto opened=s.profile().opened;
        QTimer::singleShot(2500,[] {if(auto* d=qobject_cast<QDialog*>(QApplication::activeModalWidget())) d->accept();});
        QTest::mouseClick(open,Qt::LeftButton);QCOMPARE(s.profile().opened,opened+1);
        // 概率实验跑完不修改游戏状态。
        QTest::mouseClick(window.findChild<QPushButton*>("nav3"),Qt::LeftButton);
        auto* lab=window.findChild<QWidget*>("labPage");QVERIFY(lab);lab->findChild<QSpinBox*>()->setValue(100);
        auto* start=window.findChild<QPushButton*>("labStart");QTest::mouseClick(start,Qt::LeftButton);QTRY_VERIFY_WITH_TIMEOUT(start->isEnabled(),3000);
        QCOMPARE(s.profile().opened,opened+1);
        const QString captures=qEnvironmentVariable("ARSENAL_CAPTURE_DIR");
        if(!captures.isEmpty()) {
            QDir().mkpath(captures);
            for(int n=0;n<4;++n) {QTest::mouseClick(window.findChild<QPushButton*>(QString("nav%1").arg(n)),Qt::LeftButton);QTest::qWait(100);QVERIFY(window.grab().save(captures+QString("/desktop-%1.png").arg(n)));}
            RevealDialog reveal({items.front()},&window);reveal.show();QTest::qWait(2300);QVERIFY(reveal.grab().save(captures+"/reveal.png"));
        }
    }
};
int main(int argc,char** argv) {
    QApplication app(argc,argv);app.setStyle("Fusion");app.setFont(QFont("Noto Sans CJK SC",10));
    QFile css(":/theme.qss");if(css.open(QIODevice::ReadOnly)) app.setStyleSheet(QString::fromUtf8(css.readAll()));
    GuiTests tests;return QTest::qExec(&tests,argc,argv);
}
#include "test.moc"
