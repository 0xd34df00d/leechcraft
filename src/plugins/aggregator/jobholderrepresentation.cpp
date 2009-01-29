#include "jobholderrepresentation.h"
#include <QtDebug>
#include "aggregator.h"

JobHolderRepresentation::JobHolderRepresentation (QObject *parent)
: QSortFilterProxyModel (parent)
{
	setDynamicSortFilter (true);
}

void JobHolderRepresentation::SelectionChanged (const QModelIndex& index)
{
	Selected_ = index;
	invalidateFilter ();
}

bool JobHolderRepresentation::filterAcceptsRow (int row,
		const QModelIndex&) const
{
	// The row won't show up anyway in the job list if it was empty, so
	// we can just check if it has unread items or selected. Later means
	// that user's just clicked last unread item there.
	
	return sourceModel ()->index (row, 1).data ().toInt () ||
		row == mapToSource (Selected_).row ();
}

