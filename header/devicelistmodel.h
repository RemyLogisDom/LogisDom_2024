#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractItemModel>
#include <QJsonValue>
#include <QTableWidgetItem>
#include <QDateTime>




class QDeviceTreeItem
{
public:
    QDeviceTreeItem(QDeviceTreeItem *parent = nullptr);
    ~QDeviceTreeItem();
    void appendChild(QDeviceTreeItem *item);
    QDeviceTreeItem *child(int row);
    QDeviceTreeItem *parent();
    int childCount() const;
    int row() const;
    void setKey(const QString& key);
    void setValue(const QVariant& value);
    void setType(const QJsonValue::Type& type);
    QString key() const;
    QVariant value() const;
    QJsonValue::Type type() const;

    //static QDeviceTreeItem* load(const QJsonValue& value, QDeviceTreeItem *parent = nullptr);
    //static void append(const QJsonValue& value, QDeviceTreeItem *parent = nullptr);

protected:

private:
    QString mKey;
    QVariant mValue;
    QJsonValue::Type mType;
    QList<QDeviceTreeItem*> mChilds;
    QDeviceTreeItem *mParent = nullptr;
};



class deviceListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit deviceListModel(QObject *parent = nullptr);
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QDeviceTreeItem *getMaster(QString &str);
    QDeviceTreeItem *getItem(QDeviceTreeItem *parent, QString &str);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
    QDeviceTreeItem *mRootItem = nullptr;
    QStringList mHeaders;

};

#endif // DEVICELISTMODEL_H
