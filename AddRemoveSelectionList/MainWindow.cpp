#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QStringList>
#include <vector>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList list;
    ui->_addRemoveWidget->setFullListCheckbox(false);
    ui->_addRemoveWidget->setValidNameCheck(true);
    list << "1.1" << "2.1"
        << "3" << "4"
        << "5" << "6"
        << "7" << "8"
        << "9" << "10"
        << "11" << "12"
        << "13" << "14"
        << "15" << "16"
        << "17" << "18"
        << "19" << "20"
        << "21" << "22"
        << "23" << "24"
        << "25" << "26"
        << "27" << "28"
        << "29" << "30"
        << "31" << "32"
        << "33" << "34"
        << "35" << "36"
        << "37" << "38"
        << "39" << "40"
        << "41" << "42"
        << "43" << "44"
        << "45" << "46"
        << "47" << "48"
        << "49" << "50"
        << "51" << "52";

    QStringList tips;
    tips << "A" << "B"
        << "C" << "D"
        << "E" << "F"
        << "G" << "H"
        << "I" << "J"
        << "K" << "L"
        << "M" << "N"
        << "O" << "P"
        << "Q" << "R"
        << "S" << "T"
        << "U" << "V"
        << "W" << "X"
        << "Y" << "Z"
        << "AA" << "AB"
        << "AC" << "AD"
        << "AE" << "AF"
        << "AG" << "AH"
        << "AI" << "AJ"
        << "AK" << "AL"
        << "AM" << "AN"
        << "AO" << "AP"
        << "AQ" << "AR"
        << "AS" << "AT"
        << "AU" << "AV"
        << "AW" << "AX"
        << "AY" << "AZ";
    std::vector<unsigned int> selectedIndex = {0,2,4,6,7,8,9};
    ui->_addRemoveWidget->setFullList(list,tips,selectedIndex);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

}
