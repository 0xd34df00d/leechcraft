#ifndef PLUGINS_AGGREGATOR_ITEMSLISTMODEL_H
#define PLUGINS_AGGREGATOR_ITEMSLISTMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <QPair>
#include "item.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class ItemsListModel : public QAbstractItemModel
			{
				Q_OBJECT

				QStringList ItemHeaders_;
				items_shorts_t CurrentItems_;
				int CurrentRow_;
				// First is ParentURL_ and second is Title_
				QPair<QString, QString> CurrentChannelHash_;
			public:
				ItemsListModel (QObject* = 0);

				int GetSelectedRow () const;
				const QPair<QString, QString>& GetHash () const;
				void SetHash (const QPair<QString, QString>&);
				void Selected (const QModelIndex&);
				void MarkItemAsUnread (const QModelIndex&);
				const ItemShort& GetItem (const QModelIndex&) const;
				bool IsItemRead (int) const;
				QStringList GetCategories (int) const;
				void Reset (const QPair<QString, QString>&);
				void ItemDataUpdated (Item_ptr);

				int columnCount (const QModelIndex& = QModelIndex ()) const;
				QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				Qt::ItemFlags flags (const QModelIndex&) const;
				QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex& = QModelIndex ()) const;
			};
		};
	};
};

#endif

