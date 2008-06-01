#ifndef ITEMBUCKET_H
#define ITEMBUCKET_H
#include <QDialog>
#include "ui_itembucket.h"
#include <boost/shared_ptr.hpp>

class Item;
class ItemModel;

class ItemBucket : public QDialog
{
	Q_OBJECT

	Ui::ItemBucket Ui_;
	ItemModel *Model_;

	ItemBucket ();
public:
	virtual ~ItemBucket ();
	static ItemBucket& Instance ();
	void AddItem (const boost::shared_ptr<Item>&);
private slots:
	void on_Items__activated (const QModelIndex&);
	void on_ActionDeleteItem__triggered ();
	void currentItemChanged (const QModelIndex&);
};

#endif

