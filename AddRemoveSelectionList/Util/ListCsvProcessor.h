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
    ~ListCsvProcessor() { ; }
    int static read(const QString &filename, QStringList &sList_raw, QStringList &sList_alias);    
    int static write(const QString&filename, const QStringList &sList_raw, const QStringList &sList_alias);

private:
    int readsListsFromFile(const QString &filename, QStringList &slist_raw, QStringList &sList_alias);
    int saveListsToFile(const QString &filename, const QStringList &sList_raw, const QStringList &sList_alias);    
    int _status = 0;
};

}

#endif // SIGNALLISTPROCESSOR_H
