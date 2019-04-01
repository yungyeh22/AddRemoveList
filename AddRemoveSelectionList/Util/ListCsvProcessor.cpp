#include "ListCsvProcessor.h"
#include <QFile>
#include <QTextStream>

namespace Util
{

ListCsvProcessor::ListCsvProcessor() {

}

ListCsvProcessor::ListCsvProcessor(const QString &filename) {
    read(filename);
}

ListCsvProcessor::ListCsvProcessor(const QStringList &sList_raw, const QStringList &sList_alias) :
    _list_raw(sList_raw),_list_alias(sList_alias) {

}

int ListCsvProcessor::read(const QString &filename) {
    if (QFile::exists(filename)) {
        _status = readsListsFromFile(filename);
    }
    else {
        _status = 1;
    }
    return _status;
}

int ListCsvProcessor::read(const QString &filename, QStringList &sList_raw, QStringList &sList_alias) {
    ListCsvProcessor p;
    int status = p.read(filename);
    p.getList(sList_raw,sList_alias);
    return status;
}

int ListCsvProcessor::write(const QString &filename){
    return saveListsToFile(filename, _list_raw, _list_alias);
}

int ListCsvProcessor::write(const QString &filename, const QStringList &sList_raw, const QStringList &sList_alias) {
    ListCsvProcessor p(sList_raw,sList_alias);
    return p.write(filename);
}

void ListCsvProcessor::getList(QStringList &sList_raw, QStringList &sList_alias) {
    if (_status == 0) {
        sList_raw = _list_raw;
        sList_alias = _list_alias;
    }
}

int ListCsvProcessor::readsListsFromFile(const QString &filename) {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textIn(&file);
        QString lineStr;
        while (textIn.readLineInto(&lineStr)) {
            QStringList strList = lineStr.split(",");
            _list_raw << strList.front();
            _list_alias << strList.back();
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
