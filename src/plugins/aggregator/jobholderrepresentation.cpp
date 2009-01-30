#include "jobholderrepresentation.h"
#include <QTimer>
#include <QtDebug>
#include "aggregator.h"

JobHolderRepresentation::JobHolderRepresentation (QObject *parent)
: QSortFilterProxyModel (parent)
{
	setDynamicSortFilter (true);

	Timer_ = new QTimer (this);
	connect (Timer_,
			SIGNAL (timeout ()),
			this,
			SLOT (popRow ()));
	Timer_->setInterval (1000);
	Timer_->start ();
}

void JobHolderRepresentation::SelectionChanged (const QModelIndex& index)
{
	if (Selected_.isValid ())
	{
		Previous_.enqueue (mapToSource (Selected_).row ());
		Timer_->stop ();
		Timer_->start ();
	}

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
		row == mapToSource (Selected_).row () ||
		Previous_.contains (row);
}

void JobHolderRepresentation::popRow ()
{
	if (Previous_.isEmpty ())
		return;

	Previous_.dequeue ();
	invalidateFilter ();
}

