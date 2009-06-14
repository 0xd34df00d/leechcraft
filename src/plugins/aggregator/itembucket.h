#ifndef PLUGINS_AGGREGATOR_ITEMBUCKET_H
#define PLUGINS_AGGREGATOR_ITEMBUCKET_H
#include <QDialog>
#include "item.h"
#include "ui_itembucket.h"

class QModelIndex;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemModel;

			class ItemBucket : public QDialog
			{
				Q_OBJECT

				ItemModel *Model_;
				Ui::ItemBucket Ui_;
			public:
				ItemBucket (QWidget* = 0);
				virtual ~ItemBucket ();
			private slots:
				void on_Items__activated (const QModelIndex&);
				void on_ActionDeleteItem__triggered ();
				void currentItemChanged (const QModelIndex&);
			};
		};
	};
};

#endif

