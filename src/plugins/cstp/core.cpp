#include "core.h"
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <boost/logic/tribool.hpp>
#include <QDir>
#include <QTimer>
#include <QMetaType>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "task.h"
#include "historymodel.h"
#include "xmlsettingsmanager.h"
#include "representationmodel.h"
#include "morphfile.h"

using LeechCraft::Util::Proxy;

Core::Core ()
: HistoryModel_ (new HistoryModel ())
, RepresentationModel_ (new RepresentationModel ())
, SaveScheduled_ (false)
{
	qRegisterMetaType<boost::intrusive_ptr<MorphFile> > ("boost::intrusive_ptr<MorphFile>");

	Headers_ << tr ("URL")
		<< tr ("State")
		<< tr ("Progress")
		<< tr ("Speed")
		<< tr ("ETA")
		<< tr ("DTA");

	RepresentationModel_->setSourceModel (this);

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
	writeSettings ();
	removeAllTriggered ();
	delete HistoryModel_;
	delete RepresentationModel_;
}

LeechCraft::Util::HistoryModel* Core::GetHistoryModel ()
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
	td.Task_.reset (new Task (url));
	QDir dir (path);
	td.File_.reset (new MorphFile (QDir::cleanPath (dir
					.filePath (filename))));
	td.Comment_ = comment;
	td.ErrorFlag_ = false;
	td.Parameters_ = tp;
	td.ID_ = IDPool_.front ();
	IDPool_.pop_front ();

	if (td.File_->exists ())
	{
		boost::logic::tribool remove = false;
		emit fileExists (&remove);
		if (remove)
		{
			if (!td.File_->resize (0))
			{
				QString msg = tr ("Could not truncate file ") +
					td.File_->errorString ();
				qWarning () << Q_FUNC_INFO << msg;
				emit error (msg);
			}
		}
		else if (!remove);
		else
		{
			IDPool_.push_front (td.ID_);
			return -1;
		}
	}

	connect (td.Task_.get (),
			SIGNAL (done (bool)),
			this,
			SLOT (done (bool)));
	connect (td.Task_.get (),
			SIGNAL (updateInterface ()),
			this,
			SLOT (updateInterface ()));

	beginInsertRows (QModelIndex (), rowCount (), rowCount ());
	ActiveTasks_.push_back (td);
	endInsertRows ();
	ScheduleSave ();
	if (!(tp & LeechCraft::NoAutostart))
		startTriggered (rowCount () - 1);
	return td.ID_;
}

qint64 Core::GetDone (int pos) const
{
	return TaskAt (pos).Task_->GetDone ();
}

qint64 Core::GetTotal (int pos) const
{
	return TaskAt (pos).Task_->GetTotal ();
}

bool Core::IsRunning (int pos) const
{
	return TaskAt (pos).Task_->IsRunning ();
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

QAbstractItemModel* Core::GetRepresentationModel ()
{
	return RepresentationModel_;
}

void Core::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	NetworkAccessManager_ = manager;
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return NetworkAccessManager_;
}

int Core::columnCount (const QModelIndex&) const
{
	return Headers_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (role == Qt::DisplayRole)
	{
		TaskDescr td = TaskAt (index.row ());
		boost::intrusive_ptr<Task> task = td.Task_;
		switch (index.column ())
		{
			case HURL:
				return task->GetURL ();
			case HState:
				return td.ErrorFlag_ ?
					task->GetErrorString () : task->GetState ();
			case HProgress:
				{
					qint64 done = task->GetDone (),
						   total = task->GetTotal ();
					int progress = total ? done * 100 / total : 0;
					if (done > -1)
						return QString (tr ("%1% (%2 of %3)"))
							.arg (progress)
							.arg (Proxy::Instance ()->MakePrettySize (done))
							.arg (Proxy::Instance ()->MakePrettySize (total));
					else
						return QString ("");
				}
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

Qt::ItemFlags Core::flags (const QModelIndex&) const
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

QModelIndex Core::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& parent) const
{
	return parent.isValid () ? 0 : ActiveTasks_.size ();
}

void Core::removeTriggered (int i)
{
	tasks_t::iterator it = ActiveTasks_.begin ();
	std::advance (it, i);
	Remove (it);
}

void Core::removeAllTriggered (int)
{
	while (ActiveTasks_.size ())
		removeTriggered (0);
}

void Core::startTriggered (int i)
{
	TaskDescr selected = TaskAt (i);
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

void Core::stopTriggered (int i)
{
	TaskDescr selected = TaskAt (i);
	if (!selected.Task_->IsRunning ())
		return;
	selected.Task_->Stop ();
	selected.File_->close ();
}

void Core::startAllTriggered (int)
{
	for (int i = 0, size = ActiveTasks_.size (); i < size; ++i)
		startTriggered (i);
}

void Core::stopAllTriggered (int)
{
	for (int i = 0, size = ActiveTasks_.size (); i < size; ++i)
		stopTriggered (i);
}

void Core::done (bool err)
{
	tasks_t::iterator taskdscr = FindTask (sender ());
	if (taskdscr == ActiveTasks_.end ())
		return;

	int id = taskdscr->ID_;
	QString filename = taskdscr->File_->fileName ();
	QString url = taskdscr->Task_->GetURL ();
	QString errorStr = taskdscr->Task_->GetErrorString ();

	taskdscr->File_->close ();

	if (!err)
	{
		if (!(taskdscr->Parameters_ & LeechCraft::DoNotSaveInHistory))
			AddToHistory (taskdscr);
		if (!(taskdscr->Parameters_ & LeechCraft::DoNotNotifyUser))
			emit downloadFinished (filename +
					QString ("\n") + url);
		Remove (taskdscr);
		emit taskFinished (id);
		LeechCraft::DownloadEntity e =
		{
			filename.toUtf8 (),
			QString (),
			QString (),
			taskdscr->Parameters_
		};
		qDebug () << (e.Parameters_ & LeechCraft::FromUserInitiated);
		emit gotEntity (e);
	}
	else
	{
		taskdscr->ErrorFlag_ = true;
		emit error (errorStr);
		emit taskError (id, IDownload::EUnknown);
		if (taskdscr->Parameters_ & LeechCraft::NotPersistent)
			Remove (taskdscr);
	}
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
	int taskIndex = 0;
	for (tasks_t::const_iterator i = ActiveTasks_.begin (),
			begin = ActiveTasks_.begin (),
			end = ActiveTasks_.end (); i != end; ++i)
	{
		if (i->Parameters_ & LeechCraft::NotPersistent)
			continue;

		settings.setArrayIndex (taskIndex++);
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
		td.Task_.reset (new Task ());
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
		td.File_.reset (new MorphFile (filename));

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
			default:
				return false;
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
	IDPool_.push_front (it->ID_);
	beginRemoveRows (QModelIndex (), dst, dst);
	ActiveTasks_.erase (it);
	endRemoveRows ();
	ScheduleSave ();
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

Core::tasks_t::const_reference Core::TaskAt (int pos) const
{
	tasks_t::const_iterator begin = ActiveTasks_.begin ();
	std::advance (begin, pos);
	return *begin;
}

Core::tasks_t::reference Core::TaskAt (int pos)
{
	tasks_t::iterator begin = ActiveTasks_.begin ();
	std::advance (begin, pos);
	return *begin;
}

