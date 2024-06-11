#include "devicelistmodel.h"


QDeviceTreeItem::QDeviceTreeItem(QDeviceTreeItem *parent)
{
    mParent = parent;
}

QDeviceTreeItem::~QDeviceTreeItem()
{
    qDeleteAll(mChilds);
}

void QDeviceTreeItem::appendChild(QDeviceTreeItem *item)
{
    mChilds.append(item);
}


QDeviceTreeItem *QDeviceTreeItem::child(int row)
{
    return mChilds.value(row);
}

QDeviceTreeItem *QDeviceTreeItem::parent()
{
    return mParent;
}

int QDeviceTreeItem::childCount() const
{
    return mChilds.count();
}

int QDeviceTreeItem::row() const
{
    if (mParent)
        return mParent->mChilds.indexOf(const_cast<QDeviceTreeItem*>(this));
    return 0;
}

void QDeviceTreeItem::setKey(const QString &key)
{
    mKey = key;
}

void QDeviceTreeItem::setValue(const QVariant &value)
{
    mValue = value;
}

void QDeviceTreeItem::setType(const QJsonValue::Type &type)
{
    mType = type;
}

QJsonValue::Type QDeviceTreeItem::type() const
{
    return mType;
}

QString QDeviceTreeItem::key() const
{
    return mKey;
}

QVariant QDeviceTreeItem::value() const
{
    return mValue;
}


deviceListModel::deviceListModel(QObject *parent) : QAbstractItemModel(parent) , mRootItem{new QDeviceTreeItem}
{
    mHeaders.append("RomID");
    mHeaders.append("Name");
    if (!mRootItem) mRootItem = new QDeviceTreeItem();

}


QVariant deviceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal)
        return mHeaders.value(section);
    else
        return {};
}



QModelIndex deviceListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    QDeviceTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<QDeviceTreeItem*>(parent.internalPointer());

    QDeviceTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return {};
}



QModelIndex deviceListModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    QDeviceTreeItem *childItem = static_cast<QDeviceTreeItem*>(index.internalPointer());
    QDeviceTreeItem *parentItem = childItem->parent();

    if (parentItem == mRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int deviceListModel::rowCount(const QModelIndex &parent) const
{
    QDeviceTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = mRootItem;
    else
        parentItem = static_cast<QDeviceTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}


int deviceListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}



QVariant deviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    QDeviceTreeItem *item = static_cast<QDeviceTreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return QString("%1").arg(item->key());

        if (index.column() == 1)
            return item->value();
    } else if (Qt::EditRole == role) {
        if (index.column() == 1)
            return item->value();
    }

    return {};
}



bool deviceListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int col = index.column();
    if (Qt::EditRole == role) {
        if (col == 1) {
            QDeviceTreeItem *item = static_cast<QDeviceTreeItem*>(index.internalPointer());
            item->setValue(value);
            emit dataChanged(index, index, {Qt::EditRole});
            return true;
        }
    }
    return false;
}


QDeviceTreeItem *deviceListModel::getMaster(QString &str)
{
    for (int n = 0; n<rowCount(); n++) {
        QModelIndex m = index(n, 0);
        if (m.data().toString() == str) {
            QDeviceTreeItem *item = static_cast<QDeviceTreeItem*>(m.internalPointer());
            return item;
        }
    }
    QDeviceTreeItem *item = new QDeviceTreeItem(mRootItem);
    item->setKey(str);
    mRootItem->appendChild(item);
    return item;
}



QDeviceTreeItem *deviceListModel::getItem(QDeviceTreeItem *parent, QString &str)
{

    for (int n = 0; n<parent->childCount(); n++) {
        QDeviceTreeItem *item = parent->child(n);
        if (item->key() == str) {
            return item;
        }
    }
    QDeviceTreeItem *item = new QDeviceTreeItem(parent);
    item->setKey(str);
    parent->appendChild(item);
    return item;
}

