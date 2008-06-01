#ifndef ITEMMODEL_H
#define ITEMMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <deque>
#include <boost/shared_ptr.hpp>

class Item;

class ItemModel : public QAbstractItemModel
{
	Q_OBJECT

	std::deque<boost::shared_ptr<Item> > Items_;
	QStringList ItemHeaders_;
public:
	ItemModel (QObject* = 0);
	virtual ~ItemModel ();

	void AddItem (const boost::shared_ptr<Item>&);
	void RemoveItem (const QModelIndex&);
	void Activated (const QModelIndex&) const;
	QString GetDescription (const QModelIndex&) const;

    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
};

#endif

