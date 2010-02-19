/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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
#include <plugininterface/util.h>
#include "task.h"
#include "xmlsettingsmanager.h"
#include "morphfile.h"
#include "addtask.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			Core::Core ()
			: SaveScheduled_ (false)
			, Toolbar_ (0)
			{
				setObjectName ("CSTP Core");
				qRegisterMetaType<boost::intrusive_ptr<MorphFile> > ("boost::intrusive_ptr<MorphFile>");
				qRegisterMetaType<QNetworkReply*> ("QNetworkReply*");
			
				Headers_ << tr ("URL")
					<< tr ("State")
					<< tr ("Progress");
			
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
					QString file = QFileInfo (entity.toString (QUrl::RemoveFragment)).fileName ();
					file.replace ('?', '_');
					if (file.isEmpty ())
						file = QString ("index_%1")
							.arg (QDateTime::currentDateTime ().toString (Qt::ISODate));
					return file;
				}
			}
			
			int Core::AddTask (LeechCraft::DownloadEntity& e)
			{
				QUrl entity = e.Entity_.toUrl ();
				QNetworkReply *rep = e.Entity_.value<QNetworkReply*> ();
				QStringList tags = e.Additional_ [" Tags"].toStringList ();
				if (rep)
				{
					QFileInfo fi (e.Location_);
					QString dir = fi.dir ().path ();
					QUrl source = e.Additional_ ["SourceURL"].toUrl ();
					QString file = MakeFilename (source);
			
					if (fi.isDir ())
						dir = e.Location_;
			
					return AddTask (rep,
							dir,
							file,
							QString (),
							tags,
							e.Parameters_);
				}
				else
				{
					if (e.Parameters_ & LeechCraft::FromUserInitiated &&
							e.Location_.isEmpty ())
					{
						CSTP::AddTask at (entity, e.Location_);
						if (at.exec () == QDialog::Rejected)
							return -1;
			
						AddTask::Task task = at.GetTask ();
			
						return AddTask (task.URL_,
								task.LocalPath_,
								task.Filename_,
								task.Comment_,
								tags,
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
								file = MakeFilename (entity);
							}
							else if (fi.isFile ());
							else
								return -1;
						}
			
						return AddTask (entity,
								dir,
								file,
								QString (),
								tags,
								e.Parameters_);
					}
				}
			}

			void Core::KillTask (int id)
			{
				for (int i = 0, size = ActiveTasks_.size (); i != size; ++i)
					if (static_cast<int> (ActiveTasks_ [i].ID_) == id)
					{
						removeTriggered (i);
						return;
					}
				qWarning () << Q_FUNC_INFO
					<< "not found"
					<< id
					<< ActiveTasks_.size ();
			}
			
			int Core::AddTask (QNetworkReply *rep,
					const QString& path,
					const QString& filename,
					const QString& comment,
					const QStringList& tags,
					LeechCraft::TaskParameters tp)
			{
				TaskDescr td;
				td.Task_.reset (new Task (rep));
			
				return AddTask (td, path, filename, comment, tags, tp);
			}
			
			int Core::AddTask (const QUrl& url,
					const QString& path,
					const QString& filename,
					const QString& comment,
					const QStringList& tags,
					LeechCraft::TaskParameters tp)
			{
				TaskDescr td;
				td.Task_.reset (new Task (url));
			
				return AddTask (td, path, filename, comment, tags, tp);
			}
			
			int Core::AddTask (TaskDescr& td,
					const QString& path,
					const QString& filename,
					const QString& comment,
					const QStringList& tags,
					LeechCraft::TaskParameters tp)
			{
				QDir dir (path);
				td.File_.reset (new MorphFile (QDir::cleanPath (dir
								.filePath (filename))));
				td.Comment_ = comment;
				td.ErrorFlag_ = false;
				td.Parameters_ = tp;
				td.ID_ = CoreProxy_->GetID ();
				td.Tags_ = tags;
			
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
						CoreProxy_->FreeID (td.ID_);
						return -1;
					}
				}

				if (tp & Internal)
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
				QUrl url = e.Entity_.toUrl ();
				return (url.isValid () &&
					(url.scheme () == "http" || url.scheme () == "https")) ||
					e.Entity_.value<QNetworkReply*> ();
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
								qint64 done = task->GetDone (),
									   total = task->GetTotal ();
								int progress = total ? done * 100 / total : 0;
								if (done > -1)
								{
									if (total > -1)
										return QString (tr ("%1% (%2 of %3 at %44)"))
											.arg (progress)
											.arg (Util::MakePrettySize (done))
											.arg (Util::MakePrettySize (total))
											.arg (Util::MakePrettySize (task->GetSpeed ()) + tr ("/s"));
									else
										return QString (tr ("%1"))
											.arg (Util::MakePrettySize (done));
								}
								else
									return QString ("");
							}
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
			
				int id = taskdscr->ID_;
				QString filename = taskdscr->File_->fileName ();
				QString url = taskdscr->Task_->GetURL ();
				QString errorStr = taskdscr->Task_->GetErrorString ();
				QStringList tags = taskdscr->Tags_;
			
				taskdscr->File_->close ();
			
				if (!err)
				{
					if (!(taskdscr->Parameters_ & LeechCraft::DoNotNotifyUser))
					{
						Notification n =
						{
							tr ("Download finished"),
							tr ("%1\n%2")
								.arg (filename)
								.arg (url),
							false,
							Notification::PInformation_
						};
						emit notify (n);
					}
					bool silence = taskdscr->Parameters_ & LeechCraft::DoNotAnnounceEntity;
					LeechCraft::TaskParameters tp = taskdscr->Parameters_;
					Remove (taskdscr);
					emit taskFinished (id);
					if (!silence)
					{
						tp |= LeechCraft::IsDownloaded;
						LeechCraft::DownloadEntity e =
							LeechCraft::Util::MakeEntity (QUrl::fromLocalFile (filename),
								url,
								tp);
						e.Additional_ [" Tags"] = tags;
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
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_CSTP");
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
				CoreProxy_->FreeID (id);
			
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
		};
	};
};

