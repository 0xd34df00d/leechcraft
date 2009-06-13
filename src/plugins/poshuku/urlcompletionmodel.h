#ifndef URLCOMPLETIONMODEL_H
#define URLCOMPLETIONMODEL_H
#include <QAbstractItemModel>
#include "historymodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class URLCompletionModel : public QAbstractItemModel
			{
				Q_OBJECT

				mutable bool Valid_;
				mutable history_items_t Items_;
				QString Base_;
			public:
				enum
				{
					RoleURL = 45
				};
				URLCompletionModel (QObject* = 0);
				virtual ~URLCompletionModel ();

			    virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
			    virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
			    virtual Qt::ItemFlags flags (const QModelIndex&) const;
			    virtual QVariant headerData (int, Qt::Orientation,
						int = Qt::DisplayRole) const;
			    virtual QModelIndex index (int, int,
						const QModelIndex& = QModelIndex()) const;
			    virtual QModelIndex parent (const QModelIndex&) const;
			    virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			public slots:
				void setBase (const QString&);
				void handleItemAdded (const HistoryItem&);
			private:
				void Populate ();
			};
		};
	};
};

#endif

