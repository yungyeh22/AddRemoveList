#include "ListCsvProcessor.h"
#include <QFile>
#include <QTextStream>

namespace Util
{

ListCsvProcessor::ListCsvProcessor() {

}

ListCsvProcessor::ListCsvProcessor(const QString &filename) : _filename(filename){

}

int ListCsvProcessor::read(const QString &filename, QStringList &sList_raw, QStringList &sList_alias) {
    ListCsvProcessor p(filename);
    return p.readsListsFromFile(sList_raw,sList_alias);
}

int ListCsvProcessor::write(const QString &filename, const QStringList &sList_raw, const QStringList &sList_alias) {
    ListCsvProcessor p(filename);
    return p.saveListsToFile(sList_raw,sList_alias);
}

int ListCsvProcessor::getLists(QStringList &sList_raw, QStringList &sList_alias) {
    return readsListsFromFile(sList_raw, sList_alias);
}

int ListCsvProcessor::writeLists(const QStringList &sList_raw, const QStringList &sList_alias) {
    return saveListsToFile(sList_raw, sList_alias);
}

bool ListCsvProcessor::isValid(const QString &filename) {
    ListCsvProcessor p(filename);
    return p.isValid();
}

bool ListCsvProcessor::isValid() {
    bool status = false;
    if (_filename!="") {
        if (QFile::exists(_filename)) {
            status = performCheck();
        }
        else {
            status = false;
        }
    }
    else {
        status = false;
    }
    return status;
}

int ListCsvProcessor::readsListsFromFile(QStringList &slist_raw, QStringList &sList_alias) {
    int status;
    QFile file(_filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textIn(&file);
        QString lineStr;
        while (textIn.readLineInto(&lineStr)) {
            QStringList strList = lineStr.split(",");
            slist_raw << strList.front();
            sList_alias << strList.back();
        }
        status = 0;
    }
    else {
        status = 1;
    }
    file.close();
    return status;
}

int ListCsvProcessor::saveListsToFile(const QStringList &sList_raw, const QStringList &sList_alias) {
    int status;
    QFile file(_filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int idx = 0 ; idx < sList_raw.size() ; ++idx) {
            out << sList_raw.at(idx) << "," << sList_alias.at(idx) << "\n";
        }
        status = 0;
    }
    else {
        status = 1;
    }
    file.close();
    return status;
}

bool ListCsvProcessor::performCheck() {
    bool status = true;
    QFile file(_filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textIn(&file);
        QString lineStr;
        while (textIn.readLineInto(&lineStr)) {
            if ((lineStr.count(",")+1) != 2) {
                status = false;
                break;
            }
        }
    }
    else {
        status = false;
    }
    file.close();
    return status;
}

}
