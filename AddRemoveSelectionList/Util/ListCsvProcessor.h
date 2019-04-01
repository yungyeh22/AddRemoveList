#ifndef SIGNALLISTPROCESSOR_H
#define SIGNALLISTPROCESSOR_H

#include <QStringList>

namespace Util
{

/*
 * ListCsvProcessor read / write selected list items from / to a CSV file. The CSV file can be used for
 * populating the selected list.
 */
class ListCsvProcessor {
public:
    ListCsvProcessor();
    ListCsvProcessor(const QString &filename);
    ListCsvProcessor(const QStringList &sList_raw, const QStringList &sList_alias);
    ~ListCsvProcessor() { ; }
    int read(const QString &filename);
    int static read(const QString &filename, QStringList &sList_raw, QStringList &sList_alias);
    int write(const QString &filename);
    int static write(const QString&filename, const QStringList &sList_raw, const QStringList &sList_alias);
    void getList(QStringList &sListraw, QStringList &sList_alias);
    void setList(const QStringList &sListraw, const QStringList &sList_alias);

private:
    int readsListsFromFile(const QString &filename);
    int saveListsToFile(const QString &filename, const QStringList &sList_raw, const QStringList &sList_alias);
    QStringList _list_raw;
    QStringList _list_alias;
    int _status = 0;
};

}

#endif // SIGNALLISTPROCESSOR_H
