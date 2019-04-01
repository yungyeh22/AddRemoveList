#include "ListCsvProcessor.h"
#include <QFile>
#include <QTextStream>

namespace Util
{

ListCsvProcessor::ListCsvProcessor() {

}

int ListCsvProcessor::read(const QString &filename, QStringList &sList_raw, QStringList &sList_alias) {
    ListCsvProcessor p;
    return p.readsListsFromFile(filename,sList_raw,sList_alias);
}

int ListCsvProcessor::write(const QString &filename, const QStringList &sList_raw, const QStringList &sList_alias) {
    ListCsvProcessor p;
    return p.saveListsToFile(filename,sList_raw,sList_alias);
}

int ListCsvProcessor::readsListsFromFile(const QString &filename, QStringList &slist_raw, QStringList &sList_alias) {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textIn(&file);
        QString lineStr;
        while (textIn.readLineInto(&lineStr)) {
            QStringList strList = lineStr.split(",");
            slist_raw << strList.front();
            sList_alias << strList.back();
        }
        file.close();
        _status = 0;
    }
    else {
        _status = 1;
    }
    return _status;
}

int ListCsvProcessor::saveListsToFile(const QString &filename, const QStringList &sList_raw, const QStringList &sList_alias) {
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int idx = 0 ; idx < sList_raw.size() ; ++idx) {
            out << sList_raw.at(idx) << "," << sList_alias.at(idx) << "\n";
        }
        file.close();
        _status = 0;
    }
    else {
        _status = 1;
    }
    return _status;
}

}
