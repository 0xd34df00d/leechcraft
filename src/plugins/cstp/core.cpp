#include "core.h"
#include <QFile>
#include <plugininterface/proxy.h>
#include "task.h"

Core::Core ()
{
	Headers_ << tr ("State") << tr ("URL") << tr ("Progress") << tr ("Speed") << tr ("ETA") << tr ("DTA");
}

Core::~Core ()
{
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
}

int Core::columnCount (const QModelIndex& parent) const
{
	return Headers_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (role == Qt::DisplayRole)
	{
		boost::shared_ptr<Task> task = RunningTasks_ [index.row ()]
			.Task_;
		switch (index.column ())
		{
			case HState:
				return task->GetState ();
			case HURL:
				return task->GetURL ();
			case HProgress:
				return QString::number (task->GetDone () * 100 /
						task->GetTotal ()) + "%";
			case HSpeed:
				return Proxy::Instance ()->
					MakePrettySize (task->GetSpeed ()) + tr ("/s");
			case HRemaining:
				{
					qint64 done = task->GetDone (),
						   total = task->GetTotal ();
					double speed = task->GetSpeed ();

					qint64 rem = (total - done) / speed;

					return Proxy::Instance ()->MakeTimeFromLong (rem);
				}
			case HDownloading:
				return Proxy::Instance ()->
					MakeTimeFromLong (task->GetTimeFromStart ());
			default:
				return QVariant ();
		}
	}
    else
        return QVariant ();
}

Qt::ItemFlags Core::flags (const QModelIndex& index) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool Core::hasChildren (const QModelIndex& index) const
{
	return !index.isValid ();
}

QVariant Core::headerData (int column, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return Headers_.at (column);
    else
        return QVariant ();
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex& index) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& parent) const
{
	return parent.isValid () ? 0 : RunningTasks_.size ();
}

