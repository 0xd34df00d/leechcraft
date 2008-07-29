#include "core.h"
#include <QFile>
#include <QDir>
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

void Core::AddJob (const QString& url,
		const QString& path,
		const QString& filename,
		const QString& comment)
{
	TaskDescr td;
	td.Task_ = boost::shared_ptr<Task> (new Task (url));
	QDir dir (path);
	td.File_ = boost::shared_ptr<QFile> (new QFile (QDir::cleanPath (dir
					.filePath (filename))));
	td.Comment_ = comment;
	td.ErrorFlag_ = false;

	connect (td.Task_.get (), SIGNAL (done (bool)), this, SLOT (done (bool)));

	// FIXME various checks for file

	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	ActiveTasks_.push_back (td);
	endInsertRows ();
}

void Core::Start (const QModelIndex& index)
{
	TaskDescr selected = ActiveTasks_.at (index.row ());
	selected.File_->open (QIODevice::ReadWrite);
	selected.Task_->Start (selected.File_);
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
		boost::shared_ptr<Task> task = ActiveTasks_ [index.row ()]
			.Task_;
		switch (index.column ())
		{
			case HState:
				return task->GetState ();
			case HURL:
				return task->GetURL ();
			case HProgress:
				return "Delegate isn't installed";
			case HSpeed:
				return task->IsRunning () ? Proxy::Instance ()->
					MakePrettySize (task->GetSpeed ()) + tr ("/s") :
					QVariant ();
			case HRemaining:
				{
					if (!task->IsRunning ())
						return QVariant ();

					qint64 done = task->GetDone (),
						   total = task->GetTotal ();
					double speed = task->GetSpeed ();

					qint64 rem = (total - done) / speed;

					return Proxy::Instance ()->MakeTimeFromLong (rem);
				}
			case HDownloading:
				return task->IsRunning () ? Proxy::Instance ()->
					MakeTimeFromLong (task->GetTimeFromStart ()) :
					QVariant ();;
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
	return parent.isValid () ? 0 : ActiveTasks_.size ();
}

qint64 Core::GetDone (int pos) const
{
	return ActiveTasks_.at (pos).Task_->GetDone ();
}

qint64 Core::GetTotal (int pos) const
{
	return ActiveTasks_.at (pos).Task_->GetTotal ();
}

bool Core::IsRunning (int pos) const
{
	return ActiveTasks_.at (pos).Task_->IsRunning ();
}

void Core::done (bool error)
{
	tasks_t::iterator taskdscr = FindTask (sender ());
	if (taskdscr == ActiveTasks_.end ())
		return;

	taskdscr->File_->close ();
	
	if (!error)
	{
		AddToHistory (taskdscr);
		Remove (taskdscr);
	}
	else
		taskdscr->ErrorFlag_ = true;
}

struct _Local::ObjectFinder
{
	QObject *Pred_;

	enum Type
	{
		TObject
	};

	Type Type_;

	ObjectFinder (QObject* task)
	: Pred_ (task)
	, Type_ (TObject)
	{
	}

	bool operator() (const Core::TaskDescr& td)
	{
		switch (Type_)
		{
			case TObject:
				return Pred_ == td.Task_.get ();
		}
	}
};

Core::tasks_t::const_iterator Core::FindTask (QObject *task) const
{
	return std::find_if (ActiveTasks_.begin (), ActiveTasks_.end (),
			_Local::ObjectFinder (task));
}

Core::tasks_t::iterator Core::FindTask (QObject *task)
{
	return std::find_if (ActiveTasks_.begin (), ActiveTasks_.end (),
			_Local::ObjectFinder (task));
}

void Core::Remove (tasks_t::iterator it)
{
	int dst = std::distance (ActiveTasks_.begin (), it);
	beginRemoveRows (QModelIndex (), dst, dst);
	ActiveTasks_.erase (it);
	endRemoveRows ();
}

void Core::AddToHistory (tasks_t::const_iterator it)
{
}

