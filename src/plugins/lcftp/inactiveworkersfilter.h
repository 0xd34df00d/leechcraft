#ifndef PLUGINS_LCFTP_INACTIVEWORKERSFILTER_H
#define PLUGINS_LCFTP_INACTIVEWORKERSFILTER_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class Core;

			class InactiveWorkersFilter : public QSortFilterProxyModel
			{
				Q_OBJECT

				Core *Core_;
			public:
				InactiveWorkersFilter (Core *parent);
			protected:
				bool filterAcceptsRow (int, const QModelIndex&) const;
			};
		};
	};
};

#endif

