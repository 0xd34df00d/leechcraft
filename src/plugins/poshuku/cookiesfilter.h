#ifndef COOKIESFILTER_H
#define COOKIESFILTER_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class CookiesFilter : public QSortFilterProxyModel
			{
				Q_OBJECT
			public:
				CookiesFilter (QObject* = 0);
			protected:
				bool filterAcceptsRow (int, const QModelIndex&) const;
			};
		};
	};
};

#endif

