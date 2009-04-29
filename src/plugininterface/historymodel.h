#ifndef PLUGININTERFACE_LEECHCRAFT_HISTORYMODEL_H
#define PLUGININTERFACE_LEECHCRAFT_HISTORYMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include "config.h"

namespace LeechCraft
{
	namespace Util
	{
		/** The base class for all history models in IJobHolder
		 * implementators. Reimplement RemoveItem() in the subclasses.
		 */
		class PLUGININTERFACE_API HistoryModel : public QAbstractItemModel
		{
			Q_OBJECT

			QStringList Headers_;
		protected:
			enum Header
			{
				HFilename
				, HPath
				, HDate
				, HTags
			};
		public:
			enum Role
			{
				RolePath = 33
			};
			HistoryModel (QObject* = 0);
			virtual ~HistoryModel ();

			virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
			virtual Qt::ItemFlags flags (const QModelIndex& = QModelIndex ()) const;
			virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
			virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
			virtual QModelIndex parent (const QModelIndex& = QModelIndex ()) const;

			/** Reimplement this function to allow LeechCraft to notify
			 * your model that the user wants to delete the item
			 * identified by the index.
			 *
			 * @param[in] index The index of the deleted item.
			 */
			virtual void RemoveItem (const QModelIndex& index) = 0;
		};
	};
};

#endif

