#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QStringList>
#include <vector>
#include <QFile>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    ui->_addRemoveWidget->setFullListCheckbox(false);
    ui->_addRemoveWidget->setValidNameCheck(true);    
    QStringList list;
    QStringList tooltip;
    std::vector<unsigned int> index;
    readcsv(list,tooltip,index);
    ui->_addRemoveWidget->setFullList(list,tooltip,index);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "Something is happening!";
}

void MainWindow::readcsv(QStringList &list, QStringList &tooltip, std::vector<unsigned int> &index) {
    QFile fp ("../foodlist.csv");
    if(fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textIn(&fp);
        QString lineStr;
        unsigned int idx = 0;
        while (textIn.readLineInto(&lineStr)) {
            QStringList strList = lineStr.split(",");
            list << strList.at(0);
            tooltip << strList.at(1);
            if (strList.at(2) == QString("1")) {
                index.push_back(idx);
            }
            idx++;
        }
    }
    fp.close();
}

