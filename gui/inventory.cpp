#include "pages.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
namespace arsenal {
namespace {
class NumericCell : public QTableWidgetItem {
public:
    NumericCell(const QString& label,double sort):QTableWidgetItem(label) { setData(Qt::UserRole+1,sort); }
    bool operator<(const QTableWidgetItem& other) const override {
        return data(Qt::UserRole+1).toDouble()<other.data(Qt::UserRole+1).toDouble();
    }
};
}
InventoryPage::InventoryPage(Session& s,QWidget* parent):QWidget(parent),session_(s) {
    auto* box=new QVBoxLayout(this);box->setContentsMargins(30,28,30,28);box->setSpacing(18);
    auto* title=new QLabel("你的军械库");title->setObjectName("pageTitle");box->addWidget(title);
    auto* sub=new QLabel("搜索、筛选并管理收藏。点击表头排序，选中一行进行交易。");sub->setObjectName("muted");box->addWidget(sub);
    auto* filters=new QHBoxLayout;
    search_=new QLineEdit;search_->setObjectName("inventorySearch");search_->setPlaceholderText("搜索武器、涂装或 StatTrak…");filters->addWidget(search_,1);
    rarity_=new QComboBox;rarity_->addItem("全部稀有度",-1);
    for(int t=0;t<6;++t) rarity_->addItem(QString::fromStdString(tierName(t)),t);
    filters->addWidget(rarity_);box->addLayout(filters);
    table_=new QTableWidget;table_->setObjectName("inventoryTable");table_->setColumnCount(7);
    table_->setHorizontalHeaderLabels({"ID","武器 / 涂装","稀有度","磨损值","模板","估值","锁定"});
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);table_->setAlternatingRowColors(true);
    table_->verticalHeader()->hide();table_->setSortingEnabled(true);box->addWidget(table_,1);
    count_=new QLabel;count_->setObjectName("muted");box->addWidget(count_);
    auto* actions=new QHBoxLayout;
    auto* lock=new QPushButton("锁定 / 解锁");lock->setObjectName("lockSelected");actions->addWidget(lock);
    auto* sell=new QPushButton("出售选中饰品");sell->setObjectName("sellSelected");actions->addWidget(sell);
    auto* low=new QPushButton("批量出售低等级");actions->addWidget(low);actions->addStretch();
    auto* exportButton=new QPushButton("导出 CSV");actions->addWidget(exportButton);box->addLayout(actions);
    connect(table_,&QTableWidget::cellDoubleClicked,this,[this]{inspect();});
    connect(search_,&QLineEdit::textChanged,this,[this]{refresh();});
    connect(rarity_,&QComboBox::currentIndexChanged,this,[this]{refresh();});
    connect(&s,&Session::changed,this,&InventoryPage::refresh);
    connect(lock,&QPushButton::clicked,this,[this]{const auto id=selectedId();if(id) session_.lockItem(id);});
    connect(sell,&QPushButton::clicked,this,[this]{
        const auto id=selectedId();if(!id) return;
        if(QMessageBox::question(this,"确认出售",QString("出售饰品 #%1？该操作会从背包移除饰品。").arg(id))==QMessageBox::Yes) session_.sellItem(id);
    });
    connect(low,&QPushButton::clicked,this,[this]{
        if(QMessageBox::question(this,"批量出售","出售所有未锁定的消费级与军规级饰品？") == QMessageBox::Yes) session_.sellLowItems();
    });
    connect(exportButton,&QPushButton::clicked,this,[this]{
        const auto path=QFileDialog::getSaveFileName(this,"导出背包","arsenal-inventory.csv","CSV (*.csv)");
        if(!path.isEmpty()) session_.exportCsv(path);
    });refresh();
}
void InventoryPage::inspect() {
    const auto id=selectedId();
    for(const auto& i:session_.profile().inventory) {
        if(i.id!=id) continue;
        QDialog dialog(this);dialog.setWindowTitle("饰品检视");dialog.resize(600,520);
        auto* box=new QVBoxLayout(&dialog);box->setContentsMargins(26,24,26,24);
        auto* name=new QLabel(QString::fromStdString(itemName(i)));name->setObjectName("sectionTitle");box->addWidget(name);
        auto* picture=new QLabel;picture->setAlignment(Qt::AlignCenter);picture->setPixmap(itemArt(i.skin,QSize(520,260)));box->addWidget(picture);
        auto* form=new QFormLayout;
        auto row=[&](const QString& key,const QString& val){form->addRow(key,new QLabel(val));};
        row("稀有度",QString::fromStdString(tierName(catalog()[static_cast<std::size_t>(i.skin)].tier)));
        row("外观",QString::fromStdString(wearName(i.wear))+" / "+QString::number(i.wear,'f',8));
        row("模板编号",QString::number(i.pattern));row("当前估值",QString::number(value(i))+" 积分");
        row("保护状态",i.locked?"已锁定":"未锁定");box->addLayout(form);
        auto* close=new QDialogButtonBox(QDialogButtonBox::Close);connect(close,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);box->addWidget(close);
        dialog.exec();break;
    }
}
std::uint64_t InventoryPage::selectedId() const {
    const int row=table_->currentRow();if(row<0) return 0;
    return table_->item(row,0)->data(Qt::UserRole).toULongLong();
}
void InventoryPage::refresh() {
    const auto old=selectedId();
    table_->setSortingEnabled(false);table_->setRowCount(0);
    const int tier=rarity_->currentData().toInt();const auto query=search_->text().trimmed();
    std::int64_t total=0;
    for(const auto& i:session_.profile().inventory) {
        const auto& skin=catalog()[static_cast<std::size_t>(i.skin)];
        const auto name=QString::fromStdString(itemName(i));
        if((tier>=0 && skin.tier!=tier) || !name.contains(query,Qt::CaseInsensitive)) continue;
        const int row=table_->rowCount();table_->insertRow(row);
        auto* id=new NumericCell(QString::number(i.id),static_cast<double>(i.id));id->setData(Qt::UserRole,QVariant::fromValue<qulonglong>(i.id));table_->setItem(row,0,id);
        auto* title=new QTableWidgetItem(name);title->setIcon(QIcon(itemArt(i.skin,QSize(180,90))));title->setForeground(tierColor(skin.tier));table_->setItem(row,1,title);
        table_->setItem(row,2,new NumericCell(QString::fromStdString(tierName(skin.tier)),skin.tier));
        table_->setItem(row,3,new NumericCell(QString::number(i.wear,'f',6),i.wear));
        table_->setItem(row,4,new NumericCell(QString::number(i.pattern),i.pattern));
        table_->setItem(row,5,new NumericCell(QString::number(value(i)),static_cast<double>(value(i))));
        table_->setItem(row,6,new QTableWidgetItem(i.locked?"已锁定":"—"));
        table_->setRowHeight(row,60);total+=value(i);
    }
    table_->setIconSize(QSize(90,45));table_->setSortingEnabled(true);
    for(int row=0;row<table_->rowCount();++row)
        if(table_->item(row,0)->data(Qt::UserRole).toULongLong()==old) table_->selectRow(row);
    count_->setText(QString("显示 %1 / %2 件收藏   ·   当前筛选估值 %3 积分")
        .arg(table_->rowCount()).arg(session_.profile().inventory.size()).arg(total));
}
}
