#include "core.h"
#include <stdexcept>
#include <numeric>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "task.h"
#include "historymodel.h"

Core::Core ()
: SaveScheduled_ (false)
{
	Headers_ << tr ("State")
		<< tr ("URL")
		<< tr ("Progress")
		<< tr ("Speed")
		<< tr ("ETA")
		<< tr ("DTA");
	HistoryModel_ = new HistoryModel (this);

	for (int i = 0; i < 65535; ++i)
		IDPool_.push_back (i);

	ReadSettings ();
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

QAbstractItemModel* Core::GetHistoryModel ()
{
	return HistoryModel_;
}

int Core::AddTask (const QString& url,
		const QString& path,
		const QString& filename,
		const QString& comment,
		LeechCraft::TaskParameters tp)
{
	TaskDescr td;
	td.Task_ = boost::shared_ptr<Task> (new Task (url));
	QDir dir (path);
	td.File_ = boost::shared_ptr<QFile> (new QFile (QDir::cleanPath (dir
					.filePath (filename))));
	td.Comment_ = comment;
	td.ErrorFlag_ = false;
	td.Parameters_ = tp;
	td.ID_ = IDPool_.front ();
	IDPool_.pop_front ();

	connect (td.Task_.get (), SIGNAL (done (bool)), this, SLOT (done (bool)));
	connect (td.Task_.get (), SIGNAL (updateInterface ()), this, SLOT (updateInterface ()));

	if (td.File_->exists ())
	{
		bool remove = false;
		emit fileExists (&remove);
	}

	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	ActiveTasks_.push_back (td);
	endInsertRows ();
	ScheduleSave ();
	if (tp & LeechCraft::Autostart)
		StartI (rowCount () - 1);
	return td.ID_;
}

void Core::RemoveTask (const QModelIndex& index)
{
	if (!index.isValid ())
		return;

	Stop (index);
	Remove (ActiveTasks_.begin () + index.row ());
}

void Core::RemoveFromHistory (const QModelIndex& index)
{
	HistoryModel_->Remove (index);
}

void Core::Start (const QModelIndex& index)
{
	StartI (index.row ());
}

void Core::StartI (int i)
{
	TaskDescr selected = ActiveTasks_.at (i);
	if (selected.Task_->IsRunning ())
		return;
	if (!selected.File_->open (QIODevice::ReadWrite))
	{
		QString msg = tr ("Could not open file ") +
			selected.File_->error ();
		qWarning () << Q_FUNC_INFO << msg;
		emit error (msg);
		return;
	}
	selected.Task_->Start (selected.File_);
}

void Core::Stop (const QModelIndex& index)
{
	StopI (index.row ());
}

void Core::StopI (int i)
{
	TaskDescr selected = ActiveTasks_.at (i);
	if (!selected.Task_->IsRunning ())
		return;
	selected.Task_->Stop ();
	selected.File_->close ();
}

void Core::RemoveAll ()
{
	// FIXME implement via RemoveTask
	for (int i = 0, size = ActiveTasks_.size (); i < size; ++i)
		StopI (i);
	tasks_t::iterator current = ActiveTasks_.begin ();
	while (current != ActiveTasks_.end ())
		current = Remove (current);
}

void Core::StartAll ()
{
	for (int i = 0, size = ActiveTasks_.size (); i < size; ++i)
		StartI (i);
}

void Core::StopAll ()
{
	for (int i = 0, size = ActiveTasks_.size (); i < size; ++i)
		StopI (i);
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

namespace _Local
{
	struct SpeedAccumulator
	{
		qint64 operator() (qint64 result, const Core::TaskDescr& td)
		{
			result += td.Task_->GetSpeed ();
			return result;
		}
	};
};

qint64 Core::GetTotalDownloadSpeed () const
{
	qint64 result = 0;
	return std::accumulate (ActiveTasks_.begin (), ActiveTasks_.end (),
			result, _Local::SpeedAccumulator ());
}

bool Core::CouldDownload (const QString& str, LeechCraft::TaskParameters)
{
	QUrl url (str);
	return url.isValid () &&
		(url.scheme () == "http" || url.scheme () == "https");
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
					MakeTimeFromLong (task->GetTimeFromStart () / 1000)
					: QVariant ();;
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

void Core::done (bool err)
{
	tasks_t::iterator taskdscr = FindTask (sender ());
	if (taskdscr == ActiveTasks_.end ())
		return;

	taskdscr->File_->close ();
	
	if (!err)
	{
		if (taskdscr->Parameters_ & LeechCraft::SaveInHistory)
			AddToHistory (taskdscr);
		emit taskFinished (taskdscr->ID_);
		emit fileDownloaded (taskdscr->File_->fileName ());
		if (taskdscr->Parameters_ & LeechCraft::DoNotNotifyUser)
			emit downloadFinished (taskdscr->File_->fileName () +
					QString ("\n") + taskdscr->Task_->GetURL ());
		Remove (taskdscr);
	}
	else
	{
		taskdscr->ErrorFlag_ = true;
		emit error (taskdscr->Task_->GetErrorString ());
		emit taskError (taskdscr->ID_, IDirectDownload::ErrorOther);
	}
	ScheduleSave ();
}

void Core::updateInterface ()
{
	tasks_t::const_iterator it = FindTask (sender ());
	if (it == ActiveTasks_.end ())
		return;

	int pos = std::distance<tasks_t::const_iterator>
		(ActiveTasks_.begin (), it);
	emit dataChanged (index (pos, 0), index (pos, columnCount () - 1));
}

void Core::writeSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CSTP");
	settings.beginWriteArray ("ActiveTasks");
	settings.remove ("");
	for (tasks_t::const_iterator i = ActiveTasks_.begin (),
			begin = ActiveTasks_.begin (),
			end = ActiveTasks_.end (); i != end; ++i)
	{
		settings.setArrayIndex (std::distance (begin, i));
		settings.setValue ("Task", i->Task_->Serialize ());
		settings.setValue ("Filename", i->File_->fileName ());
		settings.setValue ("Comment", i->Comment_);
		settings.setValue ("ErrorFlag", i->ErrorFlag_);
	}
	SaveScheduled_ = false;
	settings.endArray ();
}

void Core::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CSTP");
	int size = settings.beginReadArray ("ActiveTasks");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);

		TaskDescr td;

		QByteArray data = settings.value ("Task").toByteArray ();
		td.Task_ = boost::shared_ptr<Task> (new Task ());
		try
		{
			td.Task_->Deserialize (data);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			continue;
		}

		connect (td.Task_.get (),
				SIGNAL (done (bool)),
				this,
				SLOT (done (bool)));
		connect (td.Task_.get (),
				SIGNAL (updateInterface ()),
				this,
				SLOT (updateInterface ()));

		QString filename = settings.value ("Filename").toString ();
		td.File_ = boost::shared_ptr<QFile> (new QFile (filename));

		td.Comment_ = settings.value ("Comment").toString ();
		td.ErrorFlag_ = settings.value ("ErrorFlag").toBool ();

		ActiveTasks_.push_back (td);
	}
	SaveScheduled_ = false;
	settings.endArray ();
}

void Core::ScheduleSave ()
{
	if (SaveScheduled_)
		return;

	QTimer::singleShot (100, this, SLOT (writeSettings ()));
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

Core::tasks_t::iterator Core::Remove (tasks_t::iterator it)
{
	int dst = std::distance (ActiveTasks_.begin (), it);
	emit taskRemoved (it->ID_);
	beginRemoveRows (QModelIndex (), dst, dst);
	tasks_t::iterator result = ActiveTasks_.erase (it);
	endRemoveRows ();
	ScheduleSave ();
	return result;
}

void Core::AddToHistory (tasks_t::const_iterator it)
{
	HistoryModel::Item item;
	item.Filename_ = it->File_->fileName ();
	item.URL_ = it->Task_->GetURL ();
	item.Size_ = it->File_->size ();
	item.DateTime_ = QDateTime::currentDateTime ();
	HistoryModel_->Add (item);
}

