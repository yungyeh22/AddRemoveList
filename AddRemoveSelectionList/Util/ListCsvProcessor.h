#ifndef ListCsvProcessor_H
#define ListCsvProcessor_H

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
    ~ListCsvProcessor() { ; }
    // Set the file name for read or write
    void setFile(const QString &filename) { _filename = filename; }
    // Use the predefined file name to populate the lists.
    int getLists(QStringList &sList_raw, QStringList &sList_alias);
    // Use the predefiend file name to save the lists.
    int writeLists(const QStringList &sList_raw, const QStringList &sList_alias);
    // Validation
    bool isValid();

public: // Static
    // Static function that read the lists from a file. status = 0 means no error, otherwise, return a error code number.
    int static read(const QString &filename, QStringList &sList_raw, QStringList &sList_alias);
    // Static Write the lists to  a file. status = 0 means no error, otherwise, return a error code number.
    int static write(const QString&filename, const QStringList &sList_raw, const QStringList &sList_alias);
    // Validate a csv file
    bool static isValid(const QString &filename);

private:
    // Internal member function for reading the file
    int readsListsFromFile(QStringList &slist_raw, QStringList &sList_alias);
    // Internal member function for writing lists to a file
    int saveListsToFile(const QStringList &sList_raw, const QStringList &sList_alias);
    // Performing format check
    bool performCheck();
    // Absolute filepath
    QString _filename = "";
};

}

#endif // ListCsvProcessor_H
