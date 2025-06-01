/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <QDir>
#include <QTimer>
#include <QMetaType>
#include <QStringList>
#include <QtDebug>
#include <QDesktopServices>
#include <QToolBar>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/ijobholder.h>
#include <interfaces/an/constants.h>
#include <interfaces/idownload.h>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/util.h>
#include "task.h"
#include "xmlsettingsmanager.h"
#include "addtask.h"

Q_DECLARE_METATYPE (QNetworkReply*)
Q_DECLARE_METATYPE (QToolBar*)

extern "C"
{
	#ifdef Q_OS_WIN32
		#include <stdio.h>
		static const int LC_FILENAME_MAX = FILENAME_MAX;
	#else
		#include <limits.h>
		static const int LC_FILENAME_MAX = NAME_MAX;
	#endif
}

namespace LC
{
namespace CSTP
{
	Core::Core ()
	: Headers_ { "URL", tr ("State"), tr ("Progress") }
	{
		setObjectName ("CSTP Core");
		qRegisterMetaType<std::shared_ptr<QFile>> ("std::shared_ptr<QFile>");
		qRegisterMetaType<QNetworkReply*> ("QNetworkReply*");

		ReadSettings ();
	}

	Core& Core::Instance ()
	{
		static Core core;
		return core;
	}

	void Core::Release ()
	{
		writeSettings ();
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;

		NetworkAccessManager_ = proxy->GetNetworkAccessManager ();
		connect (NetworkAccessManager_,
				SIGNAL (finished (QNetworkReply*)),
				this,
				SLOT (finishedReply (QNetworkReply*)));
	}

	ICoreProxy_ptr Core::GetCoreProxy () const
	{
		return CoreProxy_;
	}

	void Core::SetToolbar (QToolBar *widget)
	{
		Toolbar_ = widget;
	}

	void Core::ItemSelected (const QModelIndex& i)
	{
		Selected_ = i;
	}

	namespace
	{
		QString MakeFilename (const QUrl& entity)
		{
			QFileInfo fileInfo (entity.toString (QUrl::RemoveFragment | QUrl::RemoveQuery));
			QString file = fileInfo.fileName ();
			if (file.length () >= LC_FILENAME_MAX)
			{
				QString extension (fileInfo.completeSuffix ());
				QString baseName (fileInfo.baseName ());

				// at least one character for file name and one for dot
				if (extension.length () > LC_FILENAME_MAX - 2)
				// in most cases there will be trash, but its hard to assume
				// how long extension could be. For example odf.tar.bz2
					extension.resize (LC_FILENAME_MAX - 2);
				if ((baseName.length () + extension.length ()) > (LC_FILENAME_MAX - 1))
					baseName.resize (LC_FILENAME_MAX - 1 - extension.length ());
				file = baseName + '.' + extension;
			}

			if (file.isEmpty ())
				file = QString ("index_%1")
					.arg (QDateTime::currentDateTime ().toString (Qt::ISODate));
			static const QRegularExpression restrictedChars (R"(,|=|;|:|\[|\]|\"|\*|\?|&|\||\\|/|(?:^LPT\d$)|(?:^COM\d$)|(?:^PRN$)|(?:^AUX$)|(?:^CON$)|(?:^NUL$))");
			static const QString replaceWith ('_');
			file.replace (restrictedChars, replaceWith);
			if (file != fileInfo.fileName ())
					qWarning () << Q_FUNC_INFO
							<< fileInfo.fileName ()
							<< "was corrected to:"
							<< file;
			return file;
		}

		QString MakeFilename (const Entity& e)
		{
			const QFileInfo fi (e.Location_);
			if (!fi.isDir ())
				return fi.fileName ();

			if (e.Additional_.contains ("Filename"))
				return e.Additional_ ["Filename"].toString ();

			const auto& url = e.Entity_.toUrl ();
			return MakeFilename (url.isValid () ? url : e.Additional_ ["SourceURL"].toUrl ());
		}
	}

	QFuture<IDownload::Result> Core::AddTask (const Entity& e)
	{
		auto url = e.Entity_.toUrl ();
		const auto& urlList = e.Entity_.value<QList<QUrl>> ();
		QNetworkReply *rep = e.Entity_.value<QNetworkReply*> ();
		const auto& tags = e.Additional_ [" Tags"].toStringList ();

		const QFileInfo fi (e.Location_);
		const auto& dir = fi.isDir () ? e.Location_ : fi.dir ().path ();
		const auto& file = MakeFilename (e);

		if (rep)
			return AddTask (rep,
					dir,
					file,
					QString (),
					tags,
					e.Parameters_);

		AddTask::Task task
		{
			url,
			dir,
			file,
			{}
		};

		auto mkErr = [] (auto type, const QString& msg)
		{
			return Util::MakeReadyFuture (IDownload::Result { { type, msg } });
		};

		if (e.Parameters_ & LC::FromUserInitiated &&
				e.Location_.isEmpty ())
		{
			CSTP::AddTask at (url, e.Location_);
			if (at.exec () == QDialog::Rejected)
				return mkErr (IDownload::Error::Type::UserCanceled, {});

			task = at.GetTask ();
		}

		if (!urlList.isEmpty ())
		{
			for (const auto& url : urlList)
				AddTask (url,
						dir,
						MakeFilename (url),
						{},
						tags,
						e.Additional_,
						e.Parameters_);

			return mkErr (IDownload::Error::Type::NoError, "Reporting result of urls list is not supported");
		}

		if (!dir.isEmpty ())
			return AddTask (task.URL_,
					task.LocalPath_,
					task.Filename_,
					task.Comment_,
					tags,
					e.Additional_,
					e.Parameters_);

		return mkErr (IDownload::Error::Type::LocalError, "Incorrect task parameters");
	}

	QFuture<IDownload::Result> Core::AddTask (QNetworkReply *rep,
			const QString& path,
			const QString& filename,
			const QString& comment,
			const QStringList& tags,
			LC::TaskParameters tp)
	{
		TaskDescr td;
		td.Task_.reset (new Task (rep));

		QDir dir (path);
		td.File_.reset (new QFile (QDir::cleanPath (dir.filePath (filename))));
		td.Comment_ = comment;
		td.Parameters_ = tp;
		td.Tags_ = tags;

		return AddTask (td);
	}

	QFuture<IDownload::Result> Core::AddTask (const QUrl& url,
			const QString& path,
			const QString& filename,
			const QString& comment,
			const QStringList& tags,
			const QVariantMap& params,
			LC::TaskParameters tp)
	{
		TaskDescr td;

		td.Task_.reset (new Task (url, params));

		QDir dir (path);
		td.File_.reset (new QFile (QDir::cleanPath (dir.filePath (filename))));
		td.Comment_ = comment;
		td.Parameters_ = tp;
		td.Tags_ = tags;

		return AddTask (td);
	}

	QFuture<IDownload::Result> Core::AddTask (TaskDescr& td)
	{
		td.ErrorFlag_ = false;

		if (td.File_->exists ())
		{
			auto fileExistsBehaviour = FileExistsBehaviour::Continue;
			emit fileExists (&fileExistsBehaviour);
			switch (fileExistsBehaviour)
			{
			case FileExistsBehaviour::Abort:
				return Util::MakeReadyFuture (IDownload::Result {
						Util::AsLeft,
						{
							IDownload::Error::Type::LocalError,
							"File already exists"
						}
					});
			case FileExistsBehaviour::Remove:
				if (!td.File_->resize (0))
				{
					QString msg = tr ("Could not truncate file ") +
								  td.File_->errorString ();
					qWarning () << Q_FUNC_INFO << msg;
					emit error (msg);
					return Util::MakeReadyFuture (IDownload::Result {
							Util::AsLeft,
							{
								IDownload::Error::Type::LocalError,
								"Could not truncate file"
							}
						});
				}
				break;
			case FileExistsBehaviour::Continue:
				break;
			}
		}

		if (td.Parameters_ & Internal)
			td.Task_->ForbidNameChanges ();

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
		if (!(td.Parameters_ & LC::NoAutostart))
			startTriggered (rowCount () - 1);
		return td.Task_->GetFuture ();
	}

	qint64 Core::GetTotalDownloadSpeed () const
	{
		return std::accumulate (ActiveTasks_.begin (), ActiveTasks_.end (), 0,
				[] (qint64 acc, const Core::TaskDescr& td)
					{ return acc + td.Task_->GetSpeed (); });
	}

	namespace
	{
		EntityTestHandleResult CheckUrl (const QUrl& url, const Entity& e)
		{
			if (!url.isValid ())
				return {};

			const QString& scheme = url.scheme ();
			if (scheme == "file")
				return (!(e.Parameters_ & FromUserInitiated) && !(e.Parameters_ & IsDownloaded)) ?
						EntityTestHandleResult (EntityTestHandleResult::PHigh) :
						EntityTestHandleResult ();
			else
			{
				const QStringList schemes { "http", "https" };
				return schemes.contains (url.scheme ()) ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}
		}
	}

	EntityTestHandleResult Core::CouldDownload (const Entity& e)
	{
		if (e.Entity_.value<QNetworkReply*> ())
			return EntityTestHandleResult (EntityTestHandleResult::PHigh);

		const auto& url = e.Entity_.toUrl ();
		const auto& urlList = e.Entity_.value<QList<QUrl>> ();
		if (url.isValid ())
			return CheckUrl (url, e);
		else if (!urlList.isEmpty ())
		{
			const auto& results = Util::Map (urlList,
					[&e] (const QUrl& url) { return CheckUrl (url, e); });
			const auto minPos = std::min_element (results.begin (), results.end (),
					[] (const EntityTestHandleResult& left, const EntityTestHandleResult& right)
					{
						return left.HandlePriority_ < right.HandlePriority_;
					});
			return minPos == results.end () ?
					EntityTestHandleResult {} :
					*minPos;
		}
		else
			return {};
	}

	QAbstractItemModel* Core::GetRepresentationModel ()
	{
		return this;
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
			return {};

		if (index.row () >= static_cast<int> (ActiveTasks_.size ()))
			return {};

		if (role == Qt::DisplayRole)
		{
			const auto& td = TaskAt (index.row ());
			const auto& task = td.Task_;
			switch (index.column ())
			{
			case HURL:
				return task->GetURL ();
			case HState:
			{
				if (td.ErrorFlag_)
					return task->GetErrorString ();

				if (!task->IsRunning ())
					return QVariant ();

				qint64 done = task->GetDone (),
						total = task->GetTotal ();
				double speed = task->GetSpeed ();

				qint64 rem = (total - done) / speed;

				return tr ("%1 (ETA: %2)")
					.arg (task->GetState ())
					.arg (Util::MakeTimeFromLong (rem));
			}
			case HProgress:
			{
				qint64 done = task->GetDone ();
				qint64 total = task->GetTotal ();
				int progress = total ? done * 100 / total : 0;
				if (done > -1)
				{
					if (total > -1)
						return QString (tr ("%1% (%2 of %3 at %4)"))
							.arg (progress)
							.arg (Util::MakePrettySize (done))
							.arg (Util::MakePrettySize (total))
							.arg (Util::MakePrettySize (task->GetSpeed ()) + tr ("/s"));
					else
						return QString ("%1")
							.arg (Util::MakePrettySize (done));
				}
				else
					return QString ("");
			}
			default:
				return QVariant ();
			}
		}
		else if (role == LC::RoleControls)
			return QVariant::fromValue<QToolBar*> (Toolbar_);
		else if (role == CustomDataRoles::RoleJobHolderRow)
			return QVariant::fromValue<JobHolderRow> (JobHolderRow::DownloadProgress);
		else if (role == JobHolderRole::ProcessState)
		{
			const auto& task = TaskAt (index.row ());

			auto state = ProcessStateInfo::State::Running;
			if (task.ErrorFlag_)
				state = ProcessStateInfo::State::Error;
			else if (!task.Task_->IsRunning ())
				state = ProcessStateInfo::State::Paused;

			return QVariant::fromValue<ProcessStateInfo> ({
					task.Task_->GetDone (),
					task.Task_->GetTotal (),
					task.Parameters_,
					state
				});
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
		if (i == -1)
		{
			if (!Selected_.isValid ())
				return;
			i = Selected_.row ();
		}

		tasks_t::iterator it = ActiveTasks_.begin ();
		std::advance (it, i);
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
			QString msg = tr ("Could not open file %1: %2")
				.arg (selected.File_->fileName ())
				.arg (selected.File_->error ());
			qWarning () << Q_FUNC_INFO
				<< msg;
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

		QString filename = taskdscr->File_->fileName ();
		QString url = taskdscr->Task_->GetURL ();
		if (url.size () > 50)
			url = url.left (50) + "...";
		QString errorStr = taskdscr->Task_->GetErrorString ();
		QStringList tags = taskdscr->Tags_;

		taskdscr->File_->close ();

		bool notifyUser = !(taskdscr->Parameters_ & LC::DoNotNotifyUser) &&
				!(taskdscr->Parameters_ & LC::Internal);

		if (notifyUser)
		{
			QString text = err ?
					tr ("Failed downloading %1 (%2).")
						.arg (url)
						.arg (errorStr) :
					tr ("Finished downloading %1 (%2).")
						.arg (filename)
						.arg (url);

			auto e = Util::MakeAN ("CSTP",
					text,
					err ? Priority::Critical : Priority::Info,
					"org.LeechCraft.CSTP",
					AN::CatDownloads,
					err ? AN::TypeDownloadError : AN::TypeDownloadFinished,
					"org.LC.Plugins.CSTP.DLFinished/" + url,
					QStringList { QUrl { url }.host (), filename });
			if (!err)
			{
				auto nah = new Util::NotificationActionHandler (e);
				nah->AddFunction (tr ("Handle..."),
						[this, filename]
						{
							auto e = Util::MakeEntity (QUrl::fromLocalFile (filename),
									{},
									FromUserInitiated);
							CoreProxy_->GetEntityManager ()->HandleEntity (e);
						});
				nah->AddFunction (tr ("Open externally"),
						[filename]
						{
							QDesktopServices::openUrl (QUrl::fromLocalFile (filename));
						});
				nah->AddFunction (tr ("Show folder"),
						[filename]
						{
							const auto& dirPath = QFileInfo (filename).absolutePath ();
							QDesktopServices::openUrl (QUrl::fromLocalFile (dirPath));
						});
			}

			CoreProxy_->GetEntityManager ()->HandleEntity (e);
		}

		if (!err)
		{
			bool silence = taskdscr->Parameters_ & LC::DoNotAnnounceEntity;
			auto tp = taskdscr->Parameters_;
			Remove (taskdscr);
			if (!silence)
			{
				tp |= IsDownloaded;
				auto e = Util::MakeEntity (QUrl::fromLocalFile (filename),
						{},
						tp);
				e.Additional_ [" Tags"] = tags;
				CoreProxy_->GetEntityManager ()->HandleEntity (e);
			}
		}
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "erroneous 'done' for"
					<< filename
					<< url
					<< errorStr;
			taskdscr->ErrorFlag_ = true;
			if (notifyUser)
				emit error (errorStr);
			if (taskdscr->Parameters_ & LC::NotPersistent)
				Remove (taskdscr);
		}
	}

	void Core::updateInterface ()
	{
		auto it = FindTask (sender ());
		if (it == ActiveTasks_.end ())
			return;

		int pos = std::distance<tasks_t::const_iterator> (ActiveTasks_.begin (), it);
		emit dataChanged (index (pos, 0), index (pos, columnCount () - 1));
	}

	void Core::writeSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CSTP");
		settings.beginWriteArray ("ActiveTasks");
		settings.remove ("");
		int taskIndex = 0;
		for (tasks_t::const_iterator i = ActiveTasks_.begin (),
				end = ActiveTasks_.end (); i != end; ++i)
		{
			if (i->Parameters_ & LC::NotPersistent)
				continue;

			settings.setArrayIndex (taskIndex++);
			settings.setValue ("Task", i->Task_->Serialize ());
			settings.setValue ("Filename", i->File_->fileName ());
			settings.setValue ("Comment", i->Comment_);
			settings.setValue ("ErrorFlag", i->ErrorFlag_);
			settings.setValue ("Tags", i->Tags_);
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
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_CSTP");
		int size = settings.beginReadArray ("ActiveTasks");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);

			TaskDescr td;

			QByteArray data = settings.value ("Task").toByteArray ();
			td.Task_ = std::make_shared<Task> ();
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

			td.File_ = std::make_shared<QFile> (settings.value ("Filename").toString ());

			td.Comment_ = settings.value ("Comment").toString ();
			td.ErrorFlag_ = settings.value ("ErrorFlag").toBool ();
			td.Tags_ = settings.value ("Tags").toStringList ();

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

	Core::tasks_t::const_iterator Core::FindTask (QObject *task) const
	{
		return std::find_if (ActiveTasks_.begin (), ActiveTasks_.end (),
				[task] (const Core::TaskDescr& td) { return task == td.Task_.get (); });
	}

	Core::tasks_t::iterator Core::FindTask (QObject *task)
	{
		return std::find_if (ActiveTasks_.begin (), ActiveTasks_.end (),
				[task] (const Core::TaskDescr& td) { return task == td.Task_.get (); });
	}

	void Core::Remove (tasks_t::iterator it)
	{
		int dst = std::distance (ActiveTasks_.begin (), it);
		beginRemoveRows (QModelIndex (), dst, dst);
		ActiveTasks_.erase (it);
		endRemoveRows ();

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
}
}
