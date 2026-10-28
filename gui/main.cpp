#include "window.hpp"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QDir>
int main(int argc,char** argv) {
    QApplication app(argc,argv);app.setApplicationName("Arsenal");app.setApplicationVersion("2.0");app.setStyle("Fusion");
    app.setFont(QFont("Noto Sans CJK SC",10));
    QFile theme(":/theme.qss");if(theme.open(QIODevice::ReadOnly)) app.setStyleSheet(QString::fromUtf8(theme.readAll()));
    QCommandLineParser parser;parser.addHelpOption();parser.addVersionOption();
    parser.addOption({"save","使用指定存档","path"});parser.process(app);
    QString path=parser.value("save");
    if(path.isEmpty()) path=QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/arsenal/profile.sav";
    try {
        arsenal::Session session(path.toStdString());arsenal::MainWindow window(session);window.show();return app.exec();
    } catch(const std::exception& e) { QMessageBox::critical(nullptr,"无法加载档案",QString::fromUtf8(e.what()));return 1; }
}
