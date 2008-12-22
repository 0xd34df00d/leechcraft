#ifndef LEECHCRAFT_HISTORYMODEL_H
#define LEECHCRAFT_HISTORYMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		class LEECHCRAFT_API HistoryModel : public QAbstractItemModel
		{
			Q_OBJECT

			QStringList Headers_;
		public:
			HistoryModel (QObject* = 0);
			virtual ~HistoryModel ();

			virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
			virtual Qt::ItemFlags flags (const QModelIndex& = QModelIndex ()) const;
			virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
			virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			virtual QModelIndex parent (const QModelIndex& = QModelIndex ()) const;

			virtual void RemoveItem (const QModelIndex&) = 0;
		};
	};
};

#endif

