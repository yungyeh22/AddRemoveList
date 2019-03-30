//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 10/30/2018      -       Yung-Yeh Chang   Original Release
// 10/31/2018      -       Yung-Yeh Chang   Better view experience when doing add/remove/reset
//*************************************************************************************************
#include "AddRemoveSelection.h"
#include "ui_AddRemoveSelection.h"
#include <algorithm>
#include <functional>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QSizePolicy>
#include <QRegularExpression>
#include <QPoint>
#include <QDebug>

namespace Gui
{
namespace Util
{

AddRemoveSelection::AddRemoveSelection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddRemoveSelection)
{   // Enable drag/drop
    ui->setupUi(this);
    // Retain _messageLabel size
    QSizePolicy szPolicy =  ui->_messageLabel->sizePolicy();
    szPolicy.setRetainSizeWhenHidden(true);
    ui->_messageLabel->setSizePolicy(szPolicy);
    // Checkbox
    ui->_fullListCheckBox->setHidden(true);
    ui->_fullListCheckBox->setChecked(true);
    // Available list view
    ui->_availableListView->setModel(&_availableItemModel);
    ui->_availableListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->_availableListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->_availableListView->setAutoScroll(false);
    // Selected list view
    ui->_selectedListView->setModel(&_selectedItemModel);
    ui->_selectedListView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->_selectedListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->_selectedListView->setDragDropOverwriteMode(false);
    ui->_selectedListView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->_selectedListView->setDefaultDropAction(Qt::MoveAction);
    ui->_selectedListView->setMovement(QListView::Snap);
    ui->_selectedListView->setAutoScroll(false);
    ui->_messageLabel->setHidden(true);
    // Renaming a signal
    connect(&_selectedItemModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(itemChangeCheck(QStandardItem*)));
    // Signals for Reordering
    /*
     * The strategy here is using the dropEvent from the QListView, and then connect to the inserted/removed signals
     * for managing the _selectedListIndex.
    */
    connect(&_selectedItemModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(whenRowsInserted(const QModelIndex, int, int)));
    connect(&_selectedItemModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(whenRowsRemoved(QModelIndex,int,int)));
    // Drop event signal
    ui->_selectedListView->viewport()->installEventFilter(this);    
}

AddRemoveSelection::~AddRemoveSelection()
{
    delete ui;
}

void AddRemoveSelection::setFullListCheckbox(bool checked) {
    ui->_fullListCheckBox->setChecked(checked);    
}

void AddRemoveSelection::setFullList (const QStringList &fullList) {    
    _fullList = fullList;
    _tooltipList.clear();
    _shortListIndex.clear();
    populateAvailableList();
}

void AddRemoveSelection::setFullList(const QStringList &fullList, const QStringList &tooltipList) {
    _fullList = fullList;
    _tooltipList.clear();
    if (tooltipList.size() == fullList.size()) {
        _tooltipList = tooltipList;
    }
    _shortListIndex.clear();
    populateAvailableList();
}

void AddRemoveSelection::setFullList(const QStringList &fullList, const QStringList &tooltipList, const std::vector<unsigned int> &listIndex) {
    _fullList = fullList;
    _tooltipList.clear();
    if (tooltipList.size() == fullList.size()) {
        _tooltipList = tooltipList;
    }
    setShortListIndex(listIndex);
}

void AddRemoveSelection::setShortListIndex(const std::vector<unsigned int> &listIndex) {
    _shortListIndex = listIndex;
    // Remove duplicate elements and sort
    std::sort(_shortListIndex.begin(), _shortListIndex.end());
    _shortListIndex.erase(std::unique( _shortListIndex.begin(), _shortListIndex.end() ), _shortListIndex.end());
    // Remove out of bound index
    _shortListIndex.erase(std::find_if(_shortListIndex.begin(),_shortListIndex.end(),
                    std::bind2nd(std::greater<unsigned int>(),_fullList.size()-1)),
                    _shortListIndex.end());
    // Display the checkbox if we do need to switch between lists
     if ((_shortListIndex.size() != (unsigned int)_fullList.size()) && !_fullList.empty()) {
         ui->_fullListCheckBox->setHidden(false);
         ui->_fullListCheckBox->setChecked(false);
     }
     populateAvailableList();
}

QStringList AddRemoveSelection::getShortList() {
    return reducedListByIndex(_fullList,_shortListIndex);
}

void AddRemoveSelection::readListFromFile(const QString &filename) {
    QStringList sList_raw, sList_alias;
    readSelectedSignalsListsFromFile(filename, sList_raw, sList_alias);
    loadSelectedSignalsFromLists(sList_raw, sList_alias);
}

void AddRemoveSelection::readSelectedSignalsListsFromFile(const QString &filename, QStringList &sList_raw, QStringList &sList_alias) {
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textIn(&file);
        QString lineStr;
        while (textIn.readLineInto(&lineStr)) {
            QStringList strList = lineStr.split(",");
            sList_raw << strList.front();
            sList_alias << strList.back();
        }
        file.close();
    }
    else {
        QMessageBox::critical(this,"Error", "Failed to load file!");
    }

}

void AddRemoveSelection::loadSelectedSignalsFromLists(const QStringList &sList_raw, const QStringList &sList_alias) {
    // Clear selection
    _selectedListIndex.clear();
    _selectedItemModel.clear();
    // Arrange selected items
    for (int idx = 0 ; idx < sList_raw.size() ; ++idx) {
        int nIndex = _fullList.indexOf(sList_raw.at(idx));
        if (nIndex != -1) {
            // Store index
            _selectedListIndex.push_back(nIndex);
            // Add items
            QStandardItem* selectedItem = new QStandardItem();
            selectedItem->setFlags(selectedItem->flags() ^ (Qt::ItemIsDropEnabled));
            selectedItem->setToolTip(sList_raw.at(idx));
            QString varName = sList_alias.at(idx);
            if (_validNameCheck) {
                makeUnderscoreVar(varName);
            }
            selectedItem->setText(varName);
            _selectedItemModel.insertRow(idx, selectedItem);
        }
    }
    populateAvailableList();
    // Show warning message
    if (!_selectedListIndex.empty() && _validNameCheck) {
        ui->_messageLabel->setHidden(false);
    }
    else {
        ui->_messageLabel->setHidden(true);
    }
}

QStringList AddRemoveSelection::getSelectedItemList(bool raw) {
    if (raw) { // Original names
        return reducedListByIndex(_fullList,_selectedListIndex);
    }
    else { // Alias names
        QStringList selectedList;
        int nRow = ui->_selectedListView->model()->rowCount();
        for (int idx = 0 ; idx < nRow ; ++idx){
            selectedList << ui->_selectedListView->model()->index(idx,0).data().toString();
        }
        return selectedList;
    }
}

void AddRemoveSelection::setTooltips(const QStringList &tooltipList) {
    // If size doesn't match, remove the tooltips.
    if (_tooltipList.size() != _fullList.size()) {
        _tooltipList.clear();
    }
    else {
        _tooltipList = tooltipList;
    }
    populateAvailableList();
}

void AddRemoveSelection::setValidNameCheck(bool state) {
    _validNameCheck = state;
}

void AddRemoveSelection::on__fullListCheckBox_clicked() {
    populateAvailableList();
}

void AddRemoveSelection::on__addItemButton_clicked() {
    if (ui->_availableListView->selectionModel()->hasSelection()){
        addItems(ui->_availableListView->selectionModel()->selectedIndexes()); // Must use selectionModel()
    }
}

void AddRemoveSelection::on__removeItemButton_clicked() {
    if (ui->_selectedListView->selectionModel()->hasSelection()) {
        removeItems(ui->_selectedListView->selectionModel()->selectedIndexes());
    }
}

void AddRemoveSelection::on__reset_clicked() {
    if (!_selectedListIndex.empty()) {
        _selectedListIndex.clear();
        _selectedItemModel.clear();
        populateAvailableList();
        ui->_messageLabel->setHidden(true);
        ui->_availableListView->scrollToTop();
    }
}

void AddRemoveSelection::on__availableListView_doubleClicked(const QModelIndex &index) {
    addItems(QModelIndexList({index}));
}

void AddRemoveSelection::on__loadListButton_clicked() {
    QString setFilter = "Comma-Separated Values File (*.csv)";
    QString filter = setFilter + ";; All files (*.*)";
    QFileDialog loadDlg(this);
    loadDlg.setNameFilter(filter);
    loadDlg.selectNameFilter(setFilter);
    loadDlg.setAcceptMode(QFileDialog::AcceptOpen);
    loadDlg.setWindowTitle("Load Selected Signals ... ");
    if (loadDlg.exec() && !loadDlg.selectedFiles().isEmpty()) {
        QUrl url = loadDlg.selectedUrls().back();
        if (url.isValid()) {
            readListFromFile(url.toLocalFile());
        }
    }
}

void AddRemoveSelection::on__saveListButton_clicked() {
    if (_selectedListIndex.size() > 0) {
        QString setFilter = "Comma-Separated Values File (*.csv)";
        QString filter = setFilter + ";; All files (*.*)";
        QFileDialog saveDlg(this);
        saveDlg.selectFile("item_list.csv");
        saveDlg.setNameFilter(filter);
        saveDlg.selectNameFilter(setFilter);
        saveDlg.setAcceptMode(QFileDialog::AcceptSave);
        saveDlg.setWindowTitle("Save Selected Signals as... ");
        if (saveDlg.exec() && !saveDlg.selectedFiles().isEmpty()) {
            QUrl url = saveDlg.selectedUrls().front();
            if (url.isValid()) {
                QStringList sList_raw = getSelectedItemList(true);
                QStringList sList_alias = getSelectedItemList(false);
                QFile file(url.toLocalFile());
                if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream out(&file);
                    for (int idx = 0 ; idx < sList_raw.size() ; ++idx) {
                        out << sList_raw.at(idx) << "," << sList_alias.at(idx) << "\n";
                    }
                    file.close();
                }
                else {
                    QMessageBox::critical(this,"Error", "Failed to save file!");
                }
            }
        }
    }

}

bool AddRemoveSelection::isFullList() {
    bool isFull = false;
    if (ui->_fullListCheckBox->isHidden()) {
        isFull = true;
    }
    else if (ui->_fullListCheckBox->isChecked()) {
        isFull = true;
    }
    else {
        isFull = false;
    }
    return isFull;
}

QStringList AddRemoveSelection::reducedListByIndex(const QStringList &fList, const std::vector<unsigned int> index) {
    QStringList reducedList;
    for (const unsigned int &idx : index) {
        if (idx < (unsigned int)fList.size()) {
            reducedList << fList.at(idx);
        }
    }
    return reducedList;
}

void AddRemoveSelection::populateAvailableList() {
    _availableItemModel.clear();
    // Preparing the string list
    QStringList reducedList;
    QStringList reducedTooltipList;
    std::vector<unsigned int> unSelectedIndex;
    std::vector<unsigned int> sortedIndex(_selectedListIndex);
    unsigned int sIdx = 0;
    if (!_fullList.isEmpty()) {
        if (isFullList()) { // Display full list
            if (_selectedListIndex.empty()) {
                reducedList = _fullList;
                reducedTooltipList = _tooltipList;
            }
            else {
                // Construct a index based on selected item index
                std::sort(sortedIndex.begin(),sortedIndex.end());
                // Create a index with only required item
                for (unsigned int idx = 0 ; idx < (unsigned int)_fullList.size() ; ++ idx) {
                    if (!sortedIndex.empty() && (sIdx < sortedIndex.size())) {
                        if (idx == sortedIndex.at(sIdx)) {
                            sIdx++;
                            continue;
                        }
                    }
                    unSelectedIndex.push_back(idx);
                }
                reducedList = reducedListByIndex(_fullList,unSelectedIndex);
                if (!_tooltipList.empty()) {
                    reducedTooltipList = reducedListByIndex(_tooltipList,unSelectedIndex);
                }
            }
        }
        else { // Short list
            // It will be easier to use find since we've had the index for the short list.
            unSelectedIndex = _shortListIndex;
            std::sort(sortedIndex.begin(),sortedIndex.end(),std::greater<unsigned int>());
            for (const auto &idx : sortedIndex) {
                auto result = std::find(unSelectedIndex.begin(),unSelectedIndex.end(),idx);
                if (result != unSelectedIndex.end()) {
                    unSelectedIndex.erase(result);
                }
            }
            reducedList = reducedListByIndex(_fullList,unSelectedIndex);
            if (!_tooltipList.empty()) {
                reducedTooltipList = reducedListByIndex(_tooltipList,unSelectedIndex);
            }
        }

        // Now prepare the items for model
        QList<QStandardItem*> itemList;
        for (auto idx = 0 ; idx < reducedList.size() ; ++idx) {
            QStandardItem* item = new QStandardItem(reducedList.at(idx));            
            if (!reducedTooltipList.empty()) {
                item->setData(reducedTooltipList.at(idx), Qt::ToolTipRole);
            }
            itemList << item;
        }
        _availableItemModel.appendColumn(itemList);
    }
}

void AddRemoveSelection::addItems(const QModelIndexList &selections) {
    QModelIndexList leftSelections = selections;
    // Get the order of index sorted so it can be removed from the left list correctly.
    // Unfortunately, std::greater<QModelIndex> doesn't work in this case. Using a lambda function instead;
    std::sort(leftSelections.begin(), leftSelections.end(),
              [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
    std::vector<unsigned int> backSortedIndex;
    unsigned int rowCount = _selectedItemModel.rowCount();
    for (const QModelIndex &idx : leftSelections) {
        QString varName = idx.data().toString();
        QStandardItem* selectedItem = new QStandardItem();
        selectedItem->setFlags(selectedItem->flags() ^ (Qt::ItemIsDropEnabled));
        if (_validNameCheck) {
            selectedItem->setToolTip(varName);
            makeUnderscoreVar(varName);
        }
        selectedItem->setText(varName);
        _selectedItemModel.insertRow(rowCount, selectedItem);        
        QString escData = QRegularExpression::escape(idx.data().toString());
        backSortedIndex.push_back(_fullList.indexOf(QRegularExpression(escData))); // Save _fullList index that moved
        _availableItemModel.removeRow(idx.row()); // Remove from the highest order
    }
    // Put elements in the right list in the correct order
    std::reverse(backSortedIndex.begin(), backSortedIndex.end());
    for (const unsigned int &idx : backSortedIndex) {
        _selectedListIndex.push_back(idx);
    }
    // Show warning message
    if (!_selectedListIndex.empty() && _validNameCheck) {
        ui->_messageLabel->setHidden(false);
    }
    else {
        ui->_messageLabel->setHidden(true);
    }
}

void AddRemoveSelection::removeItems(const QModelIndexList &selections) {
    QModelIndexList rightSelections = selections;
    std::vector<unsigned int> tempInsertedIndex;
    for (const QModelIndex &idx : rightSelections) {
        if (!isFullList() &&
            !std::binary_search(_shortListIndex.begin(),_shortListIndex.end(), _selectedListIndex.at(idx.row()))) {
            continue;
        }
        else {
            tempInsertedIndex.push_back(_selectedListIndex.at(idx.row()));
            int rowNum;
            rowNum = findNextRowInAvailableList(_selectedListIndex.at(idx.row()), tempInsertedIndex);
            QString itemText = _fullList.at(_selectedListIndex.at(idx.row()));
            _availableItemModel.insertRow(rowNum, new QStandardItem(itemText));
        }
    }
    std::sort(rightSelections.begin(), rightSelections.end(),
              [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
    for (auto idx = 0 ; idx < rightSelections.size() ; ++idx) {
        int rowNum = rightSelections.at(idx).row();
        _selectedListIndex.erase(_selectedListIndex.begin() + rowNum);
        _selectedItemModel.removeRow(rowNum);
    }
    // Show warning message
    if (!_selectedListIndex.empty() && _validNameCheck) {
        ui->_messageLabel->setHidden(false);
    }
    else {
        ui->_messageLabel->setHidden(true);
    }
    ui->_availableListView->selectionModel()->select(ui->_availableListView->indexAt(
                        ui->_availableListView->viewport()->pos()),QItemSelectionModel::Select);
}

int AddRemoveSelection::findNextRowInAvailableList(int rowNum, std::vector<unsigned int> tempIndex) {
    // To put back the item to the left listview with the original position in the full list
    // First, put to the end if the item is the last in the _fulllist
    if (rowNum == ((isFullList()) ? _fullList.size()-1 : (int)_shortListIndex.back())) {
        rowNum = _availableItemModel.rowCount();
    }
    // Second, if nothing in the left listview, put the item at the first row.
    else if (_availableItemModel.rowCount() == 0) {
        rowNum = 0;
    }
    // Otherwise...
    else if (rowNum != 0) {
        // First, we find the next available index in the stored vector
        int nextRowNum = rowNum;
        for (int pIdx = 1 ; pIdx < (_fullList.size() - rowNum) ; ++pIdx) {
            if (!isFullList() &&
                !std::binary_search(_shortListIndex.begin(),_shortListIndex.end(),rowNum+pIdx)) {
                continue;
            }
            else {
                // Check the log that has been moved back to left listview
                bool foundInTempIndex = (std::find(tempIndex.begin(), tempIndex.end(), rowNum+pIdx) != tempIndex.end());
                // Then check the current _selectedListIndex. The value at the left shouldn't exist.
                bool foundInSelectedListIndex = (std::find(_selectedListIndex.begin(), _selectedListIndex.end(), rowNum+pIdx) != _selectedListIndex.end());
                // Iterate the index until we found the next nearest item (in the _fulllist) at the left listview.
                if (foundInTempIndex || !foundInSelectedListIndex) {
                    nextRowNum = rowNum + pIdx;
                    break;
                }
            }
        }
        // If the item index is the largest comparing to the item in the left listview, put it at the end.
        if (nextRowNum == rowNum) {
            rowNum = _availableItemModel.rowCount();
        }
        // Otherwise, look for the next available location in the left listview based on previous search
        // result, use the result to insert the item.
        else {
            QList<QStandardItem*> foundItems = _availableItemModel.findItems(_fullList.at(nextRowNum));
            rowNum = _availableItemModel.indexFromItem(foundItems.front()).row();
        }
    }
    return rowNum;
}

void AddRemoveSelection::makeUnderscoreVar(QString &str) {
    str.replace(QRegularExpression("[^a-zA-Z0-9_]"), QString("_"));
}

QString AddRemoveSelection::duplicateNameHandler(const QString &str) {
    unsigned int idx = 1;
    QString newStr = str;
    while (!_selectedItemModel.findItems(newStr).empty()) {
        newStr = str+QString("_%1").arg(idx);
        idx++;
    }
    return newStr;
}

void AddRemoveSelection::messageBox(const QString &title, QMessageBox::Icon icon) {
    QMessageBox msgBox;
    msgBox.setText(title);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(icon);
    msgBox.exec();
}

/*
 * The event filter is installed to QListView::viewport for identifying
 * the dropEvent().
*/
bool AddRemoveSelection::eventFilter(QObject *object, QEvent *event) {
    if (object == ui->_selectedListView->viewport() && event->type() == QEvent::Drop) {
        _itemFromMove = true;
    }
    return QObject::eventFilter(object, event);
}

/*
 * This is the SLOT handler triggered by QStandardItemModel::itemChanged()
 * Both drag/dop event and the itemChanged event would be triggered.
 * Therefore, we have to use some means to distinguish the signal sources.
 * I used `_itemFromMove` boolean that was set during the DropEvent of
 * ListViewand unset after the reorder process complete. The method was
 * implemented for handling renaming collision and incompatibility.
 * Remind that changing item's text also results in firing on the
 * itemChange SIGNAL. So I use _renameEvent to indicate the ongoing renaming
 * process when it's set.
*/
void AddRemoveSelection::itemChangeCheck(QStandardItem *item) {
    if (!_itemFromMove) {
        if (!_renameEvent) {
            _renameEvent = true;
            QString itemText = item->text();
            QString message = "";
            // Search for duplicate
            if (_selectedItemModel.findItems(itemText).size()>1) {
                QString newItemText = duplicateNameHandler(itemText);
                message += "<b>\"" + itemText + "\"</b> will be replace with <b>\"" + newItemText + "\"</b>";
                message = "<b>Duplicate name found!</b><br><br>" + message;
                messageBox(message);
                item->setText(newItemText);
            }
            // Force to replace with valid name if necessary
            else if (_validNameCheck) {
                QString validText = itemText;
                if (itemText.indexOf(QRegularExpression("[^a-zA-Z0-9_]")) != -1) {
                    makeUnderscoreVar(validText);
                    message += "<b>\"" + itemText + "\"</b> will be replaced with <b>\"" + validText + "\"</b>";
                    message = "<b>Name is invalid! Specail characters will be replaced with \"_\"</b><br><br>" + message;
                    messageBox(message);
                    item->setText(validText);
                }
            }
        }
        _renameEvent = false;
    }
}

/*
 * The QStandardItemModel::rowsInserted and QStandardItemModel::rowsRemoved SIGNALs
 * would be triggered when the reordering happens because the reorder wasn't
 * implemented as take out/put back, and yet it's an insert and delete process.
 * The purpose of implementing methods in both event is because we have to maintain
 * the _selectedListIndex which store the vector of index for selected items. *
*/


/*
 * When drag/drop reordering is in process, one of the issues of adding the index is
 * that we don't know where it's been moved from. However, I realize that, the
 * QListView::selectionModel has that information. I guess there might be other
 * approach to gain that information but this is the best I can think of. The
 * input arguments of this SLOP has no parent (parent.raw() == -1, and
 * parent == QModelIndex()) in this case. I used the first position and the number of
 * about-to-insert items (last-first+1) to find where to insert in the
 * `_selectedListIndex`. One thing I learned is that the return values of
 * selectedIndexes() could have two conditions. For one is when the selected item with
 * row index >= first; the other case is row < first. The indexes got expended due to
 * the inserted items. The actual position of the originated selected item must account
 * for that. Also the inserted items are sorted in the QListView.
*/
void AddRemoveSelection::whenRowsInserted(const QModelIndex &parent, int first, int last) {
    if (_itemFromMove) {
        _numOfMovedItem = last - first +1;
        std::vector<unsigned int> itemIndexToBeInsert;
        for (const QModelIndex &index : ui->_selectedListView->selectionModel()->selectedIndexes()) {
            if (index.row() < first) {
                itemIndexToBeInsert.push_back(_selectedListIndex.at(index.row()));
            }
            else {
                itemIndexToBeInsert.push_back(_selectedListIndex.at(index.row()-_numOfMovedItem));
            }
        }
         std::sort(itemIndexToBeInsert.begin(),itemIndexToBeInsert.end());
        _selectedListIndex.insert(_selectedListIndex.begin()+first,
                    itemIndexToBeInsert.begin(),itemIndexToBeInsert.end());
    }
}

/*
 * This might be the final step of drag/drop reorder process. The implementation here is
 * to remove the items which we've previously inserted from _selectedListIndex. The
 * index first here indicates the item indexes that is about to be removed (from first
 *  to last). Becuas we have marked the total number of items about to be removed, we
 * conclude the process when the counter (_numOfMovedItem) reaches 0.
*/
void AddRemoveSelection::whenRowsRemoved(const QModelIndex &parent, int first, int last) {
    if (_itemFromMove) {        
        if (_numOfMovedItem > 0) {
            if (first == last) {
                _selectedListIndex.erase(_selectedListIndex.begin()+first);
            }
            else {
                _selectedListIndex.erase(_selectedListIndex.begin()+first,_selectedListIndex.begin()+last+1);
            }
            _numOfMovedItem-=(last-first+1);
        }
        // Complete reorder
        if (_numOfMovedItem == 0) {
          _itemFromMove = false;
          _itemIndexToBeRemoved.clear();
          _numOfMovedItem = 0;
        }
    }
}

}
}
