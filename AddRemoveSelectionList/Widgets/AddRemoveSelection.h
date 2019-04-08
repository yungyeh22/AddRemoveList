//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 10/30/2018      -       Yung-Yeh Chang   Original Release
// 03/30/2019      -       Yung-Yeh Chang   Fixed auto scroll issue and a crash due to special characters
// 04/07/2019      -       Yung-Yeh Chang   Rewrite add/remove functions for better compatibility
//                                          and prepare for full drag/drop. Drag/drop add items completed.
//*************************************************************************************************
#ifndef ADDREMOVESELECTION_H
#define ADDREMOVESELECTION_H

#include <vector>
#include <QWidget>
#include <QStringList>
#include <QStandardItem>
#include <qstandarditemmodel.h>
#include <QAbstractItemDelegate>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class AddRemoveSelection;
}

namespace Widgets
{

/*
 * AddRemoveSelection class uses add/remove selection lists with left and right panel
 * This widget can be used in the designer by simply adding to a form by promoting
 * to this class. It has an API to access the list you want to populate and contrl the
 * objects within this widget.
 * This widget implemented with the model/view concept in QT using QListView and
 * QStandarItemModel. It supports add/remove from a full list or a short (reduced)
 * list, double-click add, drag/drop re-position, and rename in the selected view (
 * left panel. Also, the item data (text) can be saved as an CSV file for serializing
 * the selected item list in the right panel.This widget provides many useful ideas
 * for how to implement a widget to which the purpose is similar. It can be easily
 * customized to adapt into another Qt GUI design.
*/

class AddRemoveSelection : public QWidget
{
    Q_OBJECT
    enum ActionId {AddItems,RemoveItems,ReorderItems,NoAction};

public:
    explicit AddRemoveSelection(QWidget *parent = nullptr);
    ~AddRemoveSelection() override;
    // Set full/short checkbox state
    void setFullListCheckbox (bool checked);

    // Return the full list
    QStringList fullList() const { return _fullList; }

    // Set a new full list
    void setFullList (const QStringList &fullList);

    // Set a new full list with its associated tooltip list
    void setFullList (const QStringList &fullList, const QStringList &toolTipList);

    // Set a new full list with its associated tooltip list and provide an index for a shorter list
    void setFullList (const QStringList &fullList, const QStringList &toolTipList, const std::vector<int> &shortListIndex);

    // Initialize the short list with the index range of the full list.
    // Some necessary checks makes sure the range respects the full list.
    void setShortListIndex(const std::vector<int> &listIndex);

    // Return a QStringlist of the short list (from the index range of the full list)
    QStringList getShortList();

    // Read selected signal lists from a CSV file and populate the list
    void readListFromFile(const QString &filename);

    // Return a list of items that were selected
    QStringList getSelectedItemsList(bool raw = true);

    // Return the number of selected items
    int getSelectedItemCount () const { return _selectedListIndex.size(); }

    // Return the list of tooptips.
    QStringList toolTips() { return _tooltipList ; }

    // Set the tooltip list. The size of tooltip list is the same as the full list.
    void setTooltips(const QStringList &toolTipList);

    // Set to true will allow auto-replace the special characters with underscore in the right list view.
    void setValidNameCheck (bool state);

    // Get the underscore auto-replace state
    bool validNameCheck() const { return _validNameCheck; }

private slots:
    void on__fullListCheckBox_clicked();
    void on__addItemButton_clicked();
    void on__removeItemButton_clicked();
    void on__reset_clicked();
    void on__availableListView_doubleClicked(const QModelIndex &index);
    void on__loadListButton_clicked();
    void on__saveListButton_clicked();
//    void onAvailableListRowsInserted(const QModelIndex &, int first, int last);
//    void onAvailableListRowsRemoved(const QModelIndex &, int first, int last);
    void onSelectedListRowsInserted(const QModelIndex &, int first, int last);
    void onSelectedListRowsRemoved(const QModelIndex &, int first, int last);
    void onSelectedViewEditEnd(QWidget *, QAbstractItemDelegate::EndEditHint);

private:
    // Return true if it's currently showing the full list
    bool isFullList();

    // Change between full/short list
    void populateAvailableList();

    // Produce a new item for model/view
        QStandardItem* itemFactory(QString name, QString tooltip);

    // Update selected list after items add to selectedList
    void updateSelectedListForAdd(const QModelIndexList &selections, const int &index);

    // Update selected list after item reorder in the selectedList
    void updateSelectedListAfterReorder(const QModelIndexList &selections, const int &first, const int &last);

    // Update selected list after item reorder in the selectedList
    void updateSelectedListForRemove(const QModelIndexList &selections);

    // Make items
    void makeItems();

    // Add items to the right listview
    void addItems(const QModelIndexList &selections);

    // Remove items from the right listview. Put back to left listview
    void removeItems(const QModelIndexList &selections);

    // Set the selected items to be Pre-populated in the selected view
    void loadSelectedItemsFromLists(const QStringList &sList_raw, const QStringList &sList_alias);

    // Read selected items from a CSV file
    void readSelectedItemsListFromFile(const QString &filename, QStringList &sList_raw, QStringList &sList_alias);

    // Save the selected items to a file file so we can load it again
    void saveSelectedItemsListToFile(const QString &filename);

    // Return a reduced stringlist based on index
    QStringList reducedListByIndex(const QStringList &fList, const std::vector<int> index);

    // A helper function looks for the appropriate position when remove an item from selected list
    int findNextRowInAvailableList(int itemIdx, const std::vector<int> &removedList);

    // Check item name error
    void checkItemNameError(QStandardItem *item);

    // A helper function for replacing special characters with underscores
    void makeUnderscoreVar(QString &str);

    // A helper function to handle duplicate item name
    QString duplicateNameHandler(const QString &str);

    // Itme drag & drop action identifier
    ActionId currentDragDropAction();

    // Reset drag & drop action status
    void resetDragDropActionStatus();

    // Reverse sort QModelIndexList
    void revSortIndexList(QModelIndexList & indexList);

    // Display warning message
    void messageBox(const QString &title, QMessageBox::Icon icon = QMessageBox::Warning);    

private: // Vars
    Ui::AddRemoveSelection *ui;
    QStandardItemModel _availableItemModel;
    QStandardItemModel _selectedItemModel;
    QList<QStandardItem*> _fullItemsList;
    QStringList _fullList;
    QStringList _tooltipList;    
    std::vector<int> _selectedListIndex;
    std::vector<int> _shortListIndex;
    bool _validNameCheck = false;
    // Variables helps on determine current action
    bool _itemsDropInSelectedView = false;
    bool _itemsDropInAvailableView = false;
    bool _itemsSelectedInAvailableView = false;
    bool _itemsSelectedInSelectedView = false;
    int _numOfMovedItem;

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

}

#endif // ADDREMOVESELECTION_H
