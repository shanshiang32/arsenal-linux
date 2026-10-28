#include "pages.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
namespace arsenal {
StatsPage::StatsPage(Session& s,QWidget* parent):QWidget(parent),session_(s) {
    auto* v=new QVBoxLayout(this);v->setContentsMargins(30,28,30,28);v->setSpacing(16);
    auto* title=new QLabel("收藏数据与成就");title->setObjectName("pageTitle");v->addWidget(title);
    summary_=new QLabel;summary_->setObjectName("infoBox");summary_->setWordWrap(true);v->addWidget(summary_);
    chart_=new DropChart;v->addWidget(chart_,1);
    auto* bottom=new QHBoxLayout;
    auto* a=new QVBoxLayout;auto* label=new QLabel("成就进度");label->setObjectName("sectionTitle");a->addWidget(label);
    achievements_=new QTextEdit;achievements_->setReadOnly(true);a->addWidget(achievements_);bottom->addLayout(a,1);
    auto* b=new QVBoxLayout;auto* log=new QLabel("本次会话操作记录");log->setObjectName("sectionTitle");b->addWidget(log);
    journal_=new QTextEdit;journal_->setReadOnly(true);b->addWidget(journal_);bottom->addLayout(b,1);v->addLayout(bottom,1);
    connect(&s,&Session::changed,this,&StatsPage::refresh);refresh();
}
void StatsPage::refresh() {
    const auto& p=session_.profile();std::int64_t asset=0;for(const auto& i:p.inventory) asset+=value(i);
    summary_->setText(QString("累计开箱   %1     |     背包估值   %2     |     开箱支出   %3\n出售收入   %4     |     饰品收益（含持仓）   %5")
        .arg(p.opened).arg(asset).arg(p.spent).arg(p.earned).arg(p.earned+asset-p.spent));
    chart_->setData(p.drops);
    const std::array<bool,7> unlocked={p.opened>0,p.opened>=10,p.opened>=100,p.drops[4]>0,p.drops[5]>0,p.earned>=10000,p.inventory.size()>=20};
    const QStringList names={"初次出击 · 开启1箱","十连挑战 · 累计10箱","百箱老兵 · 累计100箱","红色警报 · 获得隐秘级","金色传说 · 获得传说级","交易达人 · 出售收入10000","收藏家 · 持有20件"};
    QStringList lines;for(int n=0;n<7;++n) lines<<(unlocked[static_cast<std::size_t>(n)]?"✓  ":"○  ")+names[n];
    achievements_->setPlainText(lines.join("\n\n"));journal_->setPlainText(session_.journal().join("\n"));
}
LabPage::LabPage(QWidget* parent):QWidget(parent) {
    setObjectName("labPage");auto* v=new QVBoxLayout(this);v->setContentsMargins(30,28,30,28);v->setSpacing(18);
    auto* title=new QLabel("概率实验室");title->setObjectName("pageTitle");v->addWidget(title);
    auto* desc=new QLabel("用大样本观察随机波动。独立实验不扣积分、不改变背包；白色虚线表示理论概率。");desc->setWordWrap(true);desc->setObjectName("muted");v->addWidget(desc);
    auto* controls=new QHBoxLayout;case_=new QComboBox;
    for(const auto& c:cases()) { case_->addItem(QString::fromStdString(c.name)); }
    controls->addWidget(case_);
    count_=new QSpinBox;count_->setRange(100,1000000);count_->setSingleStep(10000);count_->setValue(100000);count_->setSuffix(" 次");controls->addWidget(count_);
    start_=new QPushButton("开始实验");start_->setObjectName("labStart");controls->addWidget(start_);
    cancel_=new QPushButton("停止");cancel_->setEnabled(false);controls->addWidget(cancel_);controls->addStretch();v->addLayout(controls);
    progress_=new QProgressBar;progress_->setRange(0,100);progress_->setValue(0);v->addWidget(progress_);
    chart_=new DropChart;v->addWidget(chart_,1);
    result_=new QLabel("等待实验 · 选择样本数后开始");result_->setWordWrap(true);result_->setObjectName("infoBox");v->addWidget(result_);
    auto* note=new QLabel("实验分批在事件循环执行，保持窗口响应；停止后保留当前已完成的结果。");note->setObjectName("muted");v->addWidget(note);
    connect(&timer_,&QTimer::timeout,this,&LabPage::step);
    connect(start_,&QPushButton::clicked,this,[this]{
        done_=0;total_=0;hits_={};target_=count_->value();index_=case_->currentIndex();
        start_->setEnabled(false);cancel_->setEnabled(true);case_->setEnabled(false);count_->setEnabled(false);timer_.start(1);
    });
    connect(cancel_,&QPushButton::clicked,this,[this]{
        timer_.stop();start_->setEnabled(true);cancel_->setEnabled(false);case_->setEnabled(true);count_->setEnabled(true);
        result_->setText(result_->text()+"  ·  已停止");
    });
}
void LabPage::step() {
    const int stop=std::min(done_+1000,target_);
    for(;done_<stop;++done_) { const auto i=engine_.roll(index_);++hits_[static_cast<std::size_t>(catalog()[static_cast<std::size_t>(i.skin)].tier)];total_+=value(i); }
    progress_->setValue(static_cast<int>(100.0*done_/target_));chart_->setData(hits_,cases()[static_cast<std::size_t>(index_)].weights);
    result_->setText(QString("完成 %1 / %2 次     平均估值 %3 积分     单箱成本 %4 积分")
        .arg(done_).arg(target_).arg(done_?total_/done_:0).arg(cases()[static_cast<std::size_t>(index_)].cost));
    if(done_==target_) { timer_.stop();start_->setEnabled(true);cancel_->setEnabled(false);case_->setEnabled(true);count_->setEnabled(true); }
}
QWidget* helpPage(QWidget* parent) {
    auto* page=new QWidget(parent);auto* v=new QVBoxLayout(page);v->setContentsMargins(30,28,30,28);
    auto* title=new QLabel("关于 ARSENAL / 学习指南");title->setObjectName("pageTitle");v->addWidget(title);
    auto* text=new QTextEdit;text->setReadOnly(true);text->setHtml(
        "<h2>一个真正的 C++ 桌面项目</h2><p>Qt 6 Widgets · C++17 · GNU Make · Git</p>"
        "<h3>操作说明</h3><p>补给站选箱与数量 → 开箱动画 → 军械库管理收藏。点击每日补给领取5000虚拟积分。"
        "锁定可防止单件及批量出售。所有修改自动保存，关闭窗口无需额外操作。</p>"
        "<h3>概率与估值</h3><p>每次开箱独立随机、无保底。六档概率在补给站公开。磨损越低通常估值越高；StatTrak概率为10%。"
        "所有图形、名称、概率与估值为教学自定义，无真实支付。</p>"
        "<h3>Makefile展示</h3><p>make：编译桌面版；make test：核心、CLI与Qt交互测试。"
        "make debug：调试版。修改一个.cpp只重编译对应目标，修改头文件重建相关模块。"
        "moc处理Q_OBJECT信号槽；rcc打包QSS样式，统一由Make管理。</p>"
        "<h3>Git展示</h3><p>git log --all --graph --oneline --decorate 查看版本演进；"
        "feature/qt-desktop记录从终端项目到桌面界面的升级；feature/gui-quality记录交互测试和文档。</p>"
        "<h3>文件与隐私</h3><p>默认保存到用户数据目录的arsenal/profile.sav，兼容原终端档案。"
        "本次会话操作记录只在内存中保留。备份为.sav文件，CSV为可导出的收藏清单。"
        "Qt依赖由系统包管理器提供，未将Qt源码或二进制放入项目。</p>");v->addWidget(text,1);return page;
}
}
