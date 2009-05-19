#include "core.h"
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <boost/logic/tribool.hpp>
#include <QDir>
#include <QTimer>
#include <QMetaType>
#include <QTextCodec>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "task.h"
#include "xmlsettingsmanager.h"
#include "representationmodel.h"
#include "morphfile.h"
#include "addtask.h"

using LeechCraft::Util::Proxy;

Core::Core ()
: RepresentationModel_ (new RepresentationModel ())
, SaveScheduled_ (false)
, Toolbar_ (0)
{
	setObjectName ("CSTP Core");
	qRegisterMetaType<boost::intrusive_ptr<MorphFile> > ("boost::intrusive_ptr<MorphFile>");
	qRegisterMetaType<QNetworkReply*> ("QNetworkReply*");

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
	delete RepresentationModel_;
}

void Core::SetToolbar (QToolBar *widget)
{
	Toolbar_ = widget;
}

void Core::ItemSelected (const QModelIndex& i)
{
	Selected_ = i;
}

int Core::AddTask (LeechCraft::DownloadEntity& e)
{
	QString entity = QTextCodec::codecForName ("UTF-8")->
		toUnicode (e.Entity_);
	QNetworkReply *rep = e.Additional_.value<QNetworkReply*> ();
	if (rep)
	{
		QFileInfo fi (e.Location_);
		QString dir = fi.dir ().path ();
		QString file = QFileInfo (entity).fileName ();

		if (fi.isDir ())
			dir = e.Location_;
		if (file.isEmpty ())
			file = "index";

		return AddTask (rep,
				dir, file, QString (), e.Parameters_);
	}
	else
	{
		if (e.Parameters_ & LeechCraft::FromUserInitiated &&
				e.Location_.isEmpty ())
		{
			::AddTask at (entity, e.Location_);
			if (at.exec () == QDialog::Rejected)
				return -1;

			AddTask::Task task = at.GetTask ();

			return Core::Instance ().AddTask (task.URL_,
					task.LocalPath_,
					task.Filename_,
					task.Comment_,
					e.Parameters_);
		}
		else
		{
			QFileInfo fi (e.Location_);
			QString dir = fi.dir ().path (),
					file = fi.fileName ();

			if (!(e.Parameters_ & LeechCraft::Internal))
			{
				if (fi.isDir ())
				{
					dir = e.Location_;
					file = QFileInfo (entity).fileName ();
					if (file.isEmpty ())
						file = "index";
				}
				else if (fi.isFile ());
				else
					return -1;
			}

			return Core::Instance ().AddTask (entity,
					dir, file, QString (), e.Parameters_);
		}
	}
}

int Core::AddTask (QNetworkReply *rep,
		const QString& path,
		const QString& filename,
		const QString& comment,
		LeechCraft::TaskParameters tp)
{
	TaskDescr td;
	td.Task_.reset (new Task (rep));

	return AddTask (td, path, filename, comment, tp);
}

int Core::AddTask (const QString& url,
		const QString& path,
		const QString& filename,
		const QString& comment,
		LeechCraft::TaskParameters tp)
{
	TaskDescr td;
	QUrl u (url);
	td.Task_.reset (new Task (u));

	return AddTask (td, path, filename, comment, tp);
}

int Core::AddTask (TaskDescr& td,
		const QString& path,
		const QString& filename,
		const QString& comment,
		LeechCraft::TaskParameters tp)
{
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

bool Core::CouldDownload (const LeechCraft::DownloadEntity& e)
{
	QDataStream str (e.Entity_);
	QVariant var;
	str >> var;

	QUrl url (QTextCodec::codecForName ("UTF-8")->toUnicode (e.Entity_));
	return (url.isValid () &&
		(url.scheme () == "http" || url.scheme () == "https")) ||
		var.value<QNetworkReply*> ();
}

QAbstractItemModel* Core::GetRepresentationModel ()
{
	return RepresentationModel_;
}

void Core::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	NetworkAccessManager_ = manager;
	connect (manager,
			SIGNAL (finished (QNetworkReply*)),
			this,
			SLOT (finishedReply (QNetworkReply*)));
}

QNetworkAccessManager* Core::GetNetworkAccessManager () const
{
	return NetworkAccessManager_;
}

bool Core::HasFinishedReply (QNetworkReply *rep) const
{
	return FinishedReplies_.contains (rep);
}

void Core::RemoveFinishedReply (QNetworkReply *rep)
{
	FinishedReplies_.remove (rep);
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
					{
						if (total > -1)
							return QString (tr ("%1% (%2 of %3)"))
								.arg (progress)
								.arg (Proxy::Instance ()->MakePrettySize (done))
								.arg (Proxy::Instance ()->MakePrettySize (total));
						else
							return QString (tr ("%1"))
								.arg (Proxy::Instance ()->MakePrettySize (done));
					}
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
	else if (role == LeechCraft::RoleControls)
		return QVariant::fromValue<QToolBar*> (Toolbar_);
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
	if (i == -1)
	{
		if (!Selected_.isValid ())
			return;
		i = Selected_.row ();
	}

	tasks_t::iterator it = ActiveTasks_.begin ();
	std::advance (it, Selected_.row ());
	Remove (it);
}

void Core::removeAllTriggered ()
{
	while (ActiveTasks_.size ())
		removeTriggered (0);
}

void Core::startTriggered (int i)
{
	if (i == -1)
	{
		if (!Selected_.isValid ())
			return;
		i = Selected_.row ();
	}

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
	if (i == -1)
	{
		if (!Selected_.isValid ())
			return;
		i = Selected_.row ();
	}

	TaskDescr selected = TaskAt (i);
	if (!selected.Task_->IsRunning ())
		return;
	selected.Task_->Stop ();
	selected.File_->close ();
}

void Core::startAllTriggered ()
{
	for (int i = 0, size = ActiveTasks_.size (); i < size; ++i)
		startTriggered (i);
}

void Core::stopAllTriggered ()
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
		if (!(taskdscr->Parameters_ & LeechCraft::DoNotNotifyUser))
			emit downloadFinished (filename +
					QString ("\n") + url);
		bool silence = taskdscr->Parameters_ & LeechCraft::DoNotAnnounceEntity;
		LeechCraft::TaskParameters tp = taskdscr->Parameters_;
		Remove (taskdscr);
		emit taskFinished (id);
		if (!silence)
		{
			tp &= ~LeechCraft::IsntDownloaded;
			LeechCraft::DownloadEntity e =
			{
				filename.toUtf8 (),
				url,
				QString (),
				tp,
				QVariant ()
			};
			emit gotEntity (e);
		}
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

void Core::finishedReply (QNetworkReply *rep)
{
	FinishedReplies_.insert (rep);
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
	int id = it->ID_;
	beginRemoveRows (QModelIndex (), dst, dst);
	ActiveTasks_.erase (it);
	endRemoveRows ();
	IDPool_.push_front (id);

	ScheduleSave ();
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

