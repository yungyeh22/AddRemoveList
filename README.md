# AddRemoveList

This is my Qt project demonstrating how to do add/remove with two list panel views. 
What I learned from this project are
 - How to do model/view
 - Moving items between views
 - Drap/drop reorder
 - Installing event filter
 - Connect SIGNAL/SLOT
 - Use QModelIndex
 - Return item from selected view and back to its original order


## Here is my idea of handling drag/drop reorder action.

1. In the constructor
```C++
// Enable drag/drop
ui->_liistView->setSelectionMode(QAbstractItemView::ExtendedSelection);
ui->_liistView->setDragDropOverwriteMode(false);
ui->_liistView->setDragDropMode(QAbstractItemView::DragDrop);
ui->_liistView->setDefaultDropAction(Qt::MoveAction);
ui->_liistView->setMovement(QListView::Snap);
// Install a event filter to the QListView::viewport()
ui->_listView->viewport()->installEventFilter(this);
```
2. Because the default QStandardItem has Qt::ItemIsDropEnable flag set, we have to switch it off. So at the method which you create the items
```C++
QStandardItem* item = new QStandardItem();
item->setFlags(selectedItem->flags() ^ (Qt::ItemIsDropEnabled));
```

3. Handle the dropEvent() with the event filter. For example:
```C++
bool MyWidget::eventFilter(QObject *object, QEvent *event) {
    if (object == ui->_listView->viewport() && event->type() == QEvent::Drop) {
        // Do something when the event fired
    }
    return QObject::eventFilter(object, event);
}
```

4. Implement SIGNAL/SLOT for the `QStandardItemModel::itemChangeCheck()`, `QStandardItemModel::rowsInserted()`, or `QStandardItemModel::rowsRemoved()` if necessary as those signals would be triggered by the Drag/Drop action.
