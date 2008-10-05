#ifndef ITEMBUCKET_H
#define ITEMBUCKET_H
#include <QDialog>
#include "item.h"

class ItemModel;
class QModelIndex;

namespace Ui
{
	class ItemBucket;
};

class ItemBucket : public QDialog
{
	Q_OBJECT

	ItemModel *Model_;
	Ui::ItemBucket *Ui_;

	ItemBucket ();
public:
	virtual ~ItemBucket ();
	static ItemBucket& Instance ();
	void Release ();
	void AddItem (const Item_ptr&);
private slots:
	void on_Items__activated (const QModelIndex&);
	void on_ActionDeleteItem__triggered ();
	void currentItemChanged (const QModelIndex&);
};

#endif

