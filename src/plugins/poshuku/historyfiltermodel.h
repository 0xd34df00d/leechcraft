#ifndef PLUGINS_POSHUKU_HISTORYFILTERMODEL_H
#define PLUGINS_POSHUKU_HISTORYFILTERMODEL_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class HistoryFilterModel : public QSortFilterProxyModel
			{
				Q_OBJECT
			public:
				HistoryFilterModel (QObject* = 0);
			protected:
				virtual bool filterAcceptsRow (int, const QModelIndex&) const;
			};
		};
	};
};

#endif

