#ifndef JOBHOLDERREPRESENTATION_H
#define JOBHOLDERREPRESENTATION_H
#include <QSortFilterProxyModel>
#include <QQueue>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class JobHolderRepresentation : public QSortFilterProxyModel
			{
				Q_OBJECT

				QModelIndex Selected_;
			public:
				JobHolderRepresentation (QObject* = 0);
				void SelectionChanged (const QModelIndex&);
			protected:
				virtual bool filterAcceptsRow (int, const QModelIndex&) const;
			};
		};
	};
};

#endif

