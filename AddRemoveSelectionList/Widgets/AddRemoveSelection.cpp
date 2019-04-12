//*************************************************************************************************
//                Revision Record
//  Date         Issue      Author               Description
// ----------  ---------  ---------------  --------------------------------------------------------
// 10/30/2018      -       Yung-Yeh Chang   Original Release
// 10/31/2018      -       Yung-Yeh Chang   Better view experience when doing add/remove/reset
// 03/30/2019      -       Yung-Yeh Chang   Fixed auto scroll issue and a crash due to special characters
// 03/31/2019      -       Yung-Yeh Chang   Separate save / load items list process to a new class
// 04/07/2019      -       Yung-Yeh Chang   Rewrite add/remove functions for better compatibility
//                                          and prepare for full drag/drop. Drag/drop add items completed.
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
#include <Util/ListCsvProcessor.h>

namespace Widgets
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
    ui->_availableListView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Disable edit once for all. Otherwise, set the item flag for each item.
    ui->_availableListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->_availableListView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->_availableListView->setDefaultDropAction(Qt::MoveAction);
    ui->_availableListView->setDragDropOverwriteMode(false);
    ui->_availableListView->setMovement(QListView::Snap);
    ui->_availableListView->setAutoScroll(true);
    // Selected list view
    ui->_selectedListView->setModel(&_selectedItemModel);
    ui->_selectedListView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->_selectedListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->_selectedListView->setDragDropOverwriteMode(false);
    ui->_selectedListView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->_selectedListView->setDefaultDropAction(Qt::MoveAction);
    ui->_selectedListView->setMovement(QListView::Snap);
    ui->_selectedListView->setAutoScroll(true);
    ui->_messageLabel->setHidden(true);    

    // Signals for Reordering
    /*
     * The strategy here is using the dropEvent from the QListView, and then connect to the inserted/removed signals
     * for managing the _selectedListIndex.
    */
    connect(&_selectedItemModel,&QStandardItemModel::rowsInserted,this,&AddRemoveSelection::onSelectedListRowsInserted);
    connect(&_availableItemModel,&QStandardItemModel::rowsInserted,this,&AddRemoveSelection::onAvailableListRowsInserted);
    connect(&_availableItemModel,&QStandardItemModel::itemChanged,this,&AddRemoveSelection::onAvailableModelItemChanged);

    // Check drap / drop event signals without reimplementing the event functions
    ui->_selectedListView->viewport()->installEventFilter(this);
    ui->_availableListView->viewport()->installEventFilter(this);
    // Check edit finish signal for renaming event
    connect(ui->_selectedListView->itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &AddRemoveSelection::onSelectedViewEditEnd);
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

void AddRemoveSelection::setFullList(const QStringList &fullList, const QStringList &tooltipList, const std::vector<int> &shortListIndex) {
    _fullList = fullList;
    _tooltipList.clear();
    if (tooltipList.size() == fullList.size()) {
        _tooltipList = tooltipList;
    }
    setShortListIndex(shortListIndex);    
    populateAvailableList();
}

void AddRemoveSelection::setShortListIndex(const std::vector<int> &listIndex) {
    _shortListIndex = listIndex;
    // Remove duplicate elements and sort
    std::sort(_shortListIndex.begin(), _shortListIndex.end());
    _shortListIndex.erase(std::unique( _shortListIndex.begin(), _shortListIndex.end() ), _shortListIndex.end());
    // Remove out of bound index
    _shortListIndex.erase(std::find_if(_shortListIndex.begin(),_shortListIndex.end(),
                    std::bind2nd(std::greater<int>(),_fullList.size()-1)),
                    _shortListIndex.end());
    // Display the checkbox if we do need to switch between lists
     if ((_shortListIndex.size() != unsigned(_fullList.size())) && !_fullList.empty()) {
         ui->_fullListCheckBox->setHidden(false);
         ui->_fullListCheckBox->setChecked(false);
     }     
}

QStringList AddRemoveSelection::getShortList() {
    return reducedListByIndex(_fullList,_shortListIndex);
}

void AddRemoveSelection::readListFromFile(const QString &filename) {
    QStringList sList_raw, sList_alias;
    readSelectedItemsListFromFile(filename, sList_raw, sList_alias);
    loadSelectedItemsFromLists(sList_raw, sList_alias);
}

void AddRemoveSelection::readSelectedItemsListFromFile(const QString &filename, QStringList &sList_raw, QStringList &sList_alias) {
    int status = Util::ListCsvProcessor::read(filename,sList_raw,sList_alias);
    if (status != 0){
        QMessageBox::critical(this,"Error", "Failed to load file!");
    }
}

void AddRemoveSelection::saveSelectedItemsListToFile(const QString &filename) {
    QStringList sList_raw = getSelectedItemsList(true);
    QStringList sList_alias = getSelectedItemsList(false);
    int status = Util::ListCsvProcessor::write(filename, sList_raw, sList_alias);
    if (status != 0) {
        QMessageBox::critical(this,"Error", "Failed to save file!");
    }
}

void AddRemoveSelection::loadSelectedItemsFromLists(const QStringList &sList_raw, const QStringList &sList_alias) {
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
            QString varName = sList_alias.at(idx);
            if (_validNameCheck) {
                makeUnderscoreVar(varName);
            }
            QStandardItem* selectedItem = itemFactory(varName,sList_raw.at(idx));
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

QStringList AddRemoveSelection::getSelectedItemsList(bool raw) {
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
        auto selectionList = ui->_availableListView->selectionModel()->selectedIndexes(); // Must use selectionModel()
        sortIndexList(selectionList);
        addItems(selectionList);
    }
}

void AddRemoveSelection::on__removeItemButton_clicked() {
    if (ui->_selectedListView->selectionModel()->hasSelection()) {
        auto selectionList = ui->_selectedListView->selectionModel()->selectedIndexes();
        sortIndexList(selectionList);
        removeItems(selectionList);
    }
}

void AddRemoveSelection::on__reset_clicked() {
    _selectedListIndex.clear();    
    _selectedItemModel.clear();
    _availableItemModel.clear();
    populateAvailableList();
    ui->_messageLabel->setHidden(true);
    ui->_availableListView->scrollToTop();
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
                saveSelectedItemsListToFile(url.toLocalFile());
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

QStringList AddRemoveSelection::reducedListByIndex(const QStringList &fList, const std::vector<int> index) {
    QStringList reducedList;
    for (const int &idx : index) {
        if (idx < fList.size()) {
            reducedList << fList.at(int(idx));
        }
    }
    return reducedList;
}

void AddRemoveSelection::populateAvailableList() { //TODO: reference selectedListIndex when items in selectedListModel exist
    _availableItemModel.clear(); // Clear model
    QList<QStandardItem*> itemsList;
    for (auto index = 0 ; index < _fullList.size() ; ++index) {
        QString itemText = _fullList.at(index);
        QString itemTooltip = (_tooltipList.empty()) ? "" : _tooltipList.at(index);
        // For short list
        if (!isFullList() && !_shortListIndex.empty() && !std::binary_search(_shortListIndex.begin(),_shortListIndex.end(),index)) {
            continue;
        } // Skip selected items
        else if (std::find(_selectedListIndex.begin(),_selectedListIndex.end(),index)!=_selectedListIndex.end()) {
            continue;
        }
        itemsList << itemFactory(itemText, itemTooltip);
    }
    _availableItemModel.appendColumn(itemsList);
    ui->_availableListView->scrollToTop();
}

QStandardItem *AddRemoveSelection::itemFactory(QString name, QString tooltip) {
    QStandardItem* item = new QStandardItem(name);
    item->setDropEnabled(false);
    if (!tooltip.isEmpty()) {
        item->setData(tooltip, Qt::ToolTipRole);
    }
    return item;
}

void AddRemoveSelection::updateSelectedListForAdd(const QModelIndexList &selections, const int &index) {
    std::vector<int> itemIndexToBeInsert;
    for (const QModelIndex &index : selections) {
        QString escData = QRegularExpression::escape(index.data().toString());
        int firstLoc = _fullList.indexOf(QRegularExpression(escData));
        itemIndexToBeInsert.push_back(firstLoc); // Save _fullList index that moved
    }
    if (index == -1) {
        for (const int &index : itemIndexToBeInsert) {
            _selectedListIndex.push_back(index);
        }
    }
    else {
        _selectedListIndex.insert(_selectedListIndex.begin()+index,itemIndexToBeInsert.begin(),itemIndexToBeInsert.end());
    }
    // Warning message
    if (!_selectedListIndex.empty() && _validNameCheck) {
        ui->_messageLabel->setHidden(false);
    }
    else {
        ui->_messageLabel->setHidden(true);
    }
}

void AddRemoveSelection::updateSelectedListAfterReorder(const QModelIndexList &selections, const int &first, const int &last) {
    unsigned int numOfMovedItem = unsigned(last - first + 1);
    std::vector<int> itemIndexToBeInsert;
    for (const QModelIndex &index : selections) {
        unsigned int rowIdx = unsigned(index.row());
        if (index.row() < first) {
            itemIndexToBeInsert.push_back(_selectedListIndex.at(rowIdx));
        }
        else {
            itemIndexToBeInsert.push_back(_selectedListIndex.at(rowIdx - numOfMovedItem));
        }
    }
    _selectedListIndex.insert(_selectedListIndex.begin()+first,
                itemIndexToBeInsert.begin(),itemIndexToBeInsert.end());
    // Remove indexes from their old positions
    for (auto index = selections.rbegin() ; index != selections.rend() ; ++index) {
        int rowIdx = index->row();
        _selectedListIndex.erase(_selectedListIndex.begin() + rowIdx);
    }
}

void AddRemoveSelection::updateSelectedListForRemove(const QModelIndexList &selections) {
    for (auto index = selections.rbegin() ; index != selections.rend() ; ++index) {
        _selectedListIndex.erase(_selectedListIndex.begin() + index->row());
    }
    // Show warning message
    if (!_selectedListIndex.empty() && _validNameCheck) {
        ui->_messageLabel->setHidden(false);
    }
    else {
        ui->_messageLabel->setHidden(true);
    }
}

void AddRemoveSelection::addItems(const QModelIndexList &selections) {    
    ui->_availableListView->setAutoScroll(false);    
    // Get the order of index sorted so it can be removed in the correct order.    
    updateSelectedListForAdd(selections, -1); // Update list
    // Prepare items to be added
    int rowNum = _selectedItemModel.rowCount();
    for (auto index = selections.rbegin() ; index != selections.rend() ; ++index) {
        _selectedItemModel.insertRow(rowNum, _availableItemModel.takeItem(index->row()));
        _availableItemModel.removeRow(index->row()); // Remove from the highest order
    }
    ui->_availableListView->setAutoScroll(true);
}

void AddRemoveSelection::removeItems(const QModelIndexList &selections) {
    ui->_selectedListView->setAutoScroll(false);    
    // Get the order of index sorted so it can be removed in the correct order.    
    // Move items back to the left panel
    std::vector<int> insertedList;
    for (auto index = selections.rbegin() ; index != selections.rend() ; ++index) {
        int rowIdx = index->row();
        int itemIdx = _selectedListIndex.at(unsigned(rowIdx));
        if (!isFullList() && // If a selected-to-remove item was not exist in the short list and the left panel is showing the short list
                             // we simply remove the item from the selected view.
            !std::binary_search(_shortListIndex.begin(),_shortListIndex.end(), itemIdx)) {            
            _selectedItemModel.removeRow(rowIdx);
        }
        else { // Once we know it needs to be added back to the left panel, we find the next available location (based on its originated order)
               // in the left panel and insert it.
            insertedList.push_back(itemIdx);
            int rowNum = findNextRowInAvailableList(itemIdx, insertedList);
            _availableItemModel.insertRow(int(rowNum), _selectedItemModel.takeItem(rowIdx));
            _selectedItemModel.removeRow(rowIdx);            
        }
    }
    updateSelectedListForRemove(selections); // Update list
    // Move focus item
    ui->_availableListView->selectionModel()->select(ui->_availableListView->indexAt(
                        ui->_availableListView->viewport()->pos()),QItemSelectionModel::Clear);
    ui->_selectedListView->setAutoScroll(false);
}

void AddRemoveSelection::removeItemsWithDragDrop(const QModelIndexList &selections) {
    std::vector<int> insertedList;
    std::vector<int> toBeAddedIndexList;
    int n = selections.count();
    QList<QStandardItem*> toBeAddedItemsList;
    for (int index = (_movedItemStartIndex+n-1) ; index >= _movedItemStartIndex ; --index) {
        int rowNum = selections.at(index-_movedItemStartIndex).row();
        int indexNum = _selectedListIndex.at(unsigned(rowNum));
        if (!isFullList() &&
                !std::binary_search(_shortListIndex.begin(),_shortListIndex.end(),indexNum)) {
            n--;
        }
        else {
            toBeAddedItemsList << _availableItemModel.takeItem(index);
            insertedList.push_back(indexNum);
            toBeAddedIndexList.push_back(findNextRowInAvailableList(indexNum,insertedList));
        }
        _availableItemModel.removeRow(index);
    }
    for (int idx = 0 ; idx < n ; ++idx) {
        _availableItemModel.insertRow(toBeAddedIndexList.at(unsigned(idx)),toBeAddedItemsList.at(idx));
    }
}

int AddRemoveSelection::findNextRowInAvailableList(int itemIdx, const std::vector<int> &insertedList) {
    int nextRowNum = -1;
    // To put back the item to the left listview with the original position in the full list
    // First, put to the end if the item is the last in the list
    if (itemIdx == ((isFullList()) ? (_fullList.size()-1) : _shortListIndex.back())) {
        nextRowNum = _availableItemModel.rowCount();
    }
    // Second, if nothing in the left listview or it's a definite first item,  put the item at the first row.
    else if (itemIdx == 0 || _availableItemModel.rowCount() == 0) {
        nextRowNum = 0;
    }    
    // Otherwise...
    else {
        if (isFullList()) {
            nextRowNum = itemIdx; // Starting location in the full list
        }
        else {
            nextRowNum = int(std::distance(_shortListIndex.begin(), // Starting location in the short list
                        std::find(_shortListIndex.begin(),_shortListIndex.end(),itemIdx)));
        }
        int n = nextRowNum;
        for (int index = 0 ; index < n ; index++) {
            int nextNum = (isFullList()) ? index :_shortListIndex.at(unsigned(index));
            bool foundInSelectedList = std::find(_selectedListIndex.begin(),_selectedListIndex.end(),nextNum) != _selectedListIndex.end();
            bool foundInInsertedList = std::find(insertedList.begin(), insertedList.end(), nextNum) != insertedList.end();
            if (foundInSelectedList && !foundInInsertedList) {
                nextRowNum--;
            }
        }
    }
    return nextRowNum;
}

void AddRemoveSelection::checkItemNameError(QStandardItem *item) {
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

void AddRemoveSelection::makeUnderscoreVar(QString &str) {
    str.replace(QRegularExpression("[^a-zA-Z0-9_]"), QString("_"));
}

QString AddRemoveSelection::duplicateNameHandler(const QString &str) {
    int idx = 1;
    QString newStr = str;
    while (!_selectedItemModel.findItems(newStr).empty()) {
        newStr = str+QString("_%1").arg(idx);
        idx++;
    }
    return newStr;
}

AddRemoveSelection::ActionId AddRemoveSelection::currentDragDropAction() {
    ActionId aId;
    if (_itemsSelectedInAvailableView & _itemsDropInSelectedView) {
        aId = ActionId::AddItems;
    }
    else if (_itemsSelectedInSelectedView & _itemsDropInAvailableView) {
        aId = ActionId::RemoveItems;
    }
    else if (_itemsSelectedInSelectedView & _itemsDropInSelectedView) {
        aId = ActionId::ReorderItems;
    }
    else {
        aId = ActionId::NoAction;
    }
    return aId;
}

void AddRemoveSelection::resetDragDropActionStatus() {
    _itemsSelectedInAvailableView = false;
    _itemsSelectedInSelectedView = false;
    _itemsDropInAvailableView = false;
    _itemsDropInSelectedView = false;
}

void AddRemoveSelection::sortIndexList(QModelIndexList &indexList, bool rev) {
    if (rev) {
        // Unfortunately, std::greater<QModelIndex> doesn't work in this case so I use a lambda function instead;
        std::sort(indexList.begin(), indexList.end(), [](const QModelIndex &a, const QModelIndex &b) { return b < a; });
    }
    else {
        std::sort(indexList.begin(), indexList.end());
    }
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
 * The event filter is installed to QListView::viewport for identifying events.
*/
bool AddRemoveSelection::eventFilter(QObject *object, QEvent *event) {
    /* Use for tracking events */
    if (object == ui->_availableListView->viewport() && event->type() == QEvent::Drop) {
        _itemsDropInAvailableView = true;
        _itemsDropInSelectedView = false;
    }
    else if (object == ui->_selectedListView->viewport() && event->type() == QEvent::Drop) {
        _itemsDropInAvailableView = false;
        _itemsDropInSelectedView = true;
        ui->_availableListView->setDragDropMode(QAbstractItemView::DragDrop);
    }
    else if (object == ui->_availableListView->viewport() && event->type() == QEvent::MouseButtonPress) {
        _itemsSelectedInAvailableView = true;
        _itemsSelectedInSelectedView = false;
    }
    else if (object == ui->_selectedListView->viewport() && event->type() == QEvent::MouseButtonPress) {
        _itemsSelectedInAvailableView = false;
        _itemsSelectedInSelectedView = true;
    }
    if (event->type() == QEvent::DragEnter) { // No drop at availableListView for items from availableListView
        if ((object->parent() == ui->_availableListView) & _itemsSelectedInAvailableView) {
            ui->_availableListView->setDragDropMode(QAbstractItemView::DragOnly);
        }
        else {
            ui->_availableListView->setDragDropMode(QAbstractItemView::DragDrop);
        }
    }
    return QObject::eventFilter(object, event);
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
 * QListView::selectionModel::selectedIndexes() has that information. The returned list
 * is the list after insert.
 * Note that the input arguments of this SLOT has parent.raw() == -1, and
 * parent == QModelIndex() in this case. I used the "first" position and the number of
 * about-to-insert items (last-first+1) to find where to insert in the `_selectedListIndex`.
 * Because the indexes got expended due tothe inserted items, we need to take care two
 * different conditions, idx.row() < first and idx.row() >= first. The later one needs to
 * substract the number of items.
 * Once the insert of `_selectedListIndex` completed, all I need to do is removing the index
 * using the list from QListView::selectionModel::selectedIndexes() backward.
*/
void AddRemoveSelection::onSelectedListRowsInserted(const QModelIndex &, int first, int last) {
    if (currentDragDropAction() == ActionId::ReorderItems) {
        QModelIndexList selectedList = ui->_selectedListView->selectionModel()->selectedIndexes();
        sortIndexList(selectedList);
        updateSelectedListAfterReorder(selectedList, first, last);
        resetDragDropActionStatus();
    }
    else if (currentDragDropAction() == ActionId::AddItems) {
        QModelIndexList selectedList = ui->_availableListView->selectionModel()->selectedIndexes();
        sortIndexList(selectedList);
        updateSelectedListForAdd(selectedList,first);
        resetDragDropActionStatus();
    }
}

void AddRemoveSelection::onAvailableListRowsInserted(const QModelIndex &, int first, int last) {
    if (currentDragDropAction() == ActionId::RemoveItems) {
        _numOfMovedItem = last - first;
        _movedItemStartIndex = first;
    }
}

void AddRemoveSelection::onSelectedViewEditEnd(QWidget *, QAbstractItemDelegate::EndEditHint) {
    // TODO:Have a context menu for resetting the name
    QStandardItem * item = _selectedItemModel.itemFromIndex(ui->_selectedListView->currentIndex());
    checkItemNameError(item);
}

void AddRemoveSelection::onAvailableModelItemChanged(const QStandardItem *item) {
    if (currentDragDropAction() == ActionId::RemoveItems) {
        if (_numOfMovedItem > 0) { // Counting down. Wait for item data added.
            _numOfMovedItem--;
        }
        else {
            QModelIndexList selectedList = ui->_selectedListView->selectionModel()->selectedIndexes();
            sortIndexList(selectedList);
            removeItemsWithDragDrop(selectedList);
            updateSelectedListForRemove(selectedList);
            resetDragDropActionStatus();
        }
    }
}

}
