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
#include <QUrl>
#include <QMutexLocker>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QPushButton>
#include <QToolBar>
#include <QTimer>
#include <curl/curl.h>
#include <plugininterface/util.h>
#include "inactiveworkersfilter.h"
#include "xmlsettingsmanager.h"
#include "watchthread.h"
#include "tabmanager.h"
#include "summarytab.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Core* Core::Instance_ = 0;
			QMutex Core::InstanceMutex_;

			Core::Core ()
			: WatchThread_ (new WatchThread (this))
			, Quitting_ (false)
			, TabManager_ (new TabManager (this))
			, NumScheduledWorkers_ (0)
			, RunningHandles_ (0)
			, Toolbar_ (new QToolBar ())
			, SummaryTab_ (new SummaryTab ())
			{
				qRegisterMetaType<TaskData> ("TaskData");
				qRegisterMetaTypeStreamOperators<TaskData> ("TaskData");
				qRegisterMetaType<FetchedEntry> ("FetchedEntry");
				qRegisterMetaType<TaskData> ("LeechCraft::Plugins::LCFTP::TaskData");
				qRegisterMetaTypeStreamOperators<TaskData> ("LeechCraft::Plugins::LCFTP::TaskData");
				qRegisterMetaType<FetchedEntry> ("LeechCraft::Plugins::LCFTP::FetchedEntry");

				WorkersFilter_.reset (new InactiveWorkersFilter (this));

				{
					QMutexLocker l (&MultiHandleMutex_);
					MultiHandle_.reset (curl_multi_init (), curl_multi_cleanup);
				}
				ShareHandle_.reset (curl_share_init (), curl_share_cleanup);

				handleTotalNumWorkersChanged ();

				XmlSettingsManager::Instance ().RegisterObject ("TotalNumWorkers",
						this, "handleTotalNumWorkersChanged");
				XmlSettingsManager::Instance ().RegisterObject ("WorkersPerDomain",
						this, "handleWorkersPerDomainChanged");

				SetupToolbar ();

				handleUpdateInterface ();

				WatchThread_->start (QThread::IdlePriority);
				connect (WatchThread_,
						SIGNAL (shouldPerform ()),
						this,
						SLOT (handlePerform ()),
						Qt::QueuedConnection);

				QTimer *timer = new QTimer (this);
				timer->setInterval (500);
				connect (timer,
						SIGNAL (timeout ()),
						this,
						SLOT (handleUpdateInterface ()));
				timer->start ();

				QTimer::singleShot (5000,
						this,
						SLOT (loadTasks ()));
			}

			Core& Core::Instance ()
			{
				if (!Instance_)
				{
					QMutexLocker locker (&InstanceMutex_);
					if (!Instance_)
						Instance_ = new Core ();
				}
				return *Instance_;
			}

			void Core::Release ()
			{
				SaveTasks ();

				Quitting_ = true;

				deleteLater ();

				{
					QMutexLocker l (&MultiHandleMutex_);
					Q_FOREACH (Worker_ptr w, Workers_)
						curl_multi_remove_handle (MultiHandle_.get (),
								w->GetHandle ().get ());
				}

				WatchThread_->SetExit ();
				if (!WatchThread_->wait (600))
					WatchThread_->terminate ();

				delete WatchThread_;
			}

			void Core::SetCoreProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetCoreProxy () const
			{
				return Proxy_;
			}

			QAbstractItemModel* Core::GetModel () const
			{
				return WorkersFilter_.get ();
			}

			qint64 Core::GetDownloadSpeed () const
			{
				qint64 result = 0;
				Q_FOREACH (Worker::TaskState st, States_)
					result += st.DLSpeed_;
				return result;
			}

			qint64 Core::GetUploadSpeed () const
			{
				qint64 result = 0;
				Q_FOREACH (Worker::TaskState st, States_)
					result += st.ULSpeed_;
				return result;
			}

			TabManager* Core::GetTabManager () const
			{
				return TabManager_;
			}

			int Core::columnCount (const QModelIndex&) const
			{
				return 3;
			}

			QVariant Core::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();

				int working = Workers_.size ();
				int r = index.row ();
				int c = index.column ();

				if (role == Qt::DisplayRole)
				{
					switch (c)
					{
						case 0:
							if (r < working)
								return States_.at (r).URL_.toString ();
							else
								return Tasks_ [r - working].URL_.toString ();
						case 1:
							if (r >= working)
								return tr ("Waiting");
							else
							{
								Worker::TaskState st = States_.at (r);
								if (st.Paused_)
									return tr ("Paused");
								else if (States_.at (r).Direction_ == TaskData::DDownload)
									return tr ("Downloading at %1")
										.arg (Util::MakePrettySize (States_.at (r).DLSpeed_));
								else
									return tr ("Uploading at %1")
										.arg (Util::MakePrettySize (States_.at (r).ULSpeed_));
							}
						case 2:
							if (r >= working)
								return QVariant ();
							else
							{
								QPair<quint64, quint64> s =
									(States_.at (r).Direction_ == TaskData::DDownload ?
										States_.at (r).DL_ :
										States_.at (r).UL_);
								if (s.second)
									return tr ("%1 of %2 (%3%)")
										.arg (Util::MakePrettySize (s.first))
										.arg (Util::MakePrettySize (s.second))
										.arg (s.first * 100 / s.second);
								else
									return tr ("%1")
										.arg (Util::MakePrettySize (s.first));
							}
						default:
							return QVariant ();
					}
				}
				else if (role == RoleControls)
					return QVariant::fromValue<QToolBar*> (Toolbar_);
				else if (role == RoleAdditionalInfo)
					return QVariant::fromValue<QWidget*> (SummaryTab_);
				else if (role == RoleDownSpeedLimit)
				{
					if (r >= working)
						return 0;
					else
						return Workers_.at (r)->GetDownLimit ();
				}
				else if (role == RoleUpSpeedLimit)
				{
					if (r >= working)
						return 0;
					else
						return Workers_.at (r)->GetDownLimit ();
				}
				else if (role == RoleLog)
				{
					if (r >= working)
						return 0;
					else
						return Workers_.at (r)->GetLog ();
				}
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
				if (parent.isValid ())
					return 0;

				int result = 0;
				result += Workers_.size ();
				result += Tasks_.size ();
				return result;
			}

			bool Core::setData (const QModelIndex& index, const QVariant& value, int role)
			{
				if (!index.isValid ())
					return false;

				int r = index.row ();
				int working = Workers_.size ();
				if (r >= working)
					return false;

				if (role == RoleDownSpeedLimit)
				{
					Workers_.at (r)->SetDownLimit (value.toInt ());
					return true;
				}
				else if (role == RoleUpSpeedLimit)
				{
					Workers_.at (r)->SetUpLimit (value.toInt ());
					return true;
				}
				else
					return false;
			}

			QStringList Core::Provides () const
			{
				curl_version_info_data *data = curl_version_info (CURLVERSION_NOW);
				QStringList result ("ftp");
				if (data->age > 0)
				{
					if (data->features & CURL_VERSION_SSL)
						result << "ftps";
				}
				return result;
			}

			bool Core::IsOK (const DownloadEntity& e) const
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return false;

				QUrl url = e.Entity_.toUrl ();
				if (Provides ().contains (url.scheme ()))
					return true;
				else
					return false;
			}

			namespace
			{
				QString CheckName (const QUrl& url, const QString& location)
				{
					QFileInfo fi (location);
					QString dir, file;
					if (fi.isDir ())
						dir = fi.path ();
					else
					{
						dir = fi.dir ().path ();
						file = fi.fileName ();
					}

					if (fi.isDir ())
					{
						dir = location;
						if (file.isEmpty ())
							file = QFileInfo (url.toString (QUrl::RemoveFragment)).fileName ();

						QDir fd (dir);

						if (!file.isEmpty () &&
								fd.exists (file))
						{
							QMessageBox box (QMessageBox::Question,
									Core::tr ("LeechCraft"),
									Core::tr ("%1 already exists. What do you want to do?")
										.arg (QDir::toNativeSeparators (dir + "/" + file)));
							QPushButton *resume = box.addButton (Core::tr ("Resume"),
									QMessageBox::AcceptRole);
							QPushButton *overwrite = box.addButton (Core::tr ("Overwrite"),
									QMessageBox::DestructiveRole);
							QPushButton *rename = box.addButton (Core::tr ("Rename"),
									QMessageBox::ActionRole);
							QPushButton *cancel = box.addButton (Core::tr ("Cancel"),
									QMessageBox::RejectRole);
							box.setDefaultButton (resume);
							box.setEscapeButton (cancel);

							box.exec ();

							QAbstractButton *clicked = box.clickedButton ();
							if (clicked == overwrite)
							{
								if (!fd.remove (file))
								{
									QMessageBox::critical (0,
											Core::tr ("LeechCraft"),
											Core::tr ("Error removing %1")
												.arg (QDir::toNativeSeparators (dir + "/" + file)));
									return QString ();
								}
							}
							else if (clicked == resume)
							{
								// Do nothing
							}
							else if (clicked == rename)
							{
								while (fd.exists (file))
								{
									QString filename = QFileDialog::getSaveFileName (0,
											Core::tr ("Choose new file name"),
											dir);
									if (filename.isEmpty ())
										return QString ();
									fi = QFileInfo (filename);
									dir = fi.dir ().path ();
									file = fi.fileName ();
									fd = QDir (dir);
								}
							}
							else
								return QString ();
						}
						else if (file.isEmpty ())
						{
							QString path = url.path ();
							if (path.count ("/") >= 2)
							{
								int right = path.lastIndexOf ("/");
								int left = path.lastIndexOf ("/", -2);
								if (!dir.endsWith ("/"))
									dir += "/";
								QString add = path.mid (left + 1, right - left - 1);
								QDir rd (dir);
								rd.mkdir (add);
								dir += add;
							}
						}
					}

					return dir + "/" + file;
				}
			};

			int Core::Add (DownloadEntity e)
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return -1;

				return Add (e.Entity_.toUrl (), e.Location_,
						!(e.Parameters_ & LeechCraft::Internal));
			}

			int Core::Add (const QUrl& url, const QString& location, bool check)
			{
				QFileInfo fi (location);
				QString dir, file;
				if (fi.isDir ())
					dir = fi.path ();
				else
				{
					dir = fi.dir ().path ();
					file = fi.fileName ();
				}
	
				QString tfn = dir + "/" + file;
				if (check)
				{
					tfn = CheckName (url, location);
					if (tfn.isEmpty ())
						return -1;
				}

				TaskData td =
				{
					TaskData::DDownload,
					Proxy_->GetID (),
					url,
					tfn,
					false,
					false
				};
				QueueTask (td);
				return td.ID_;
			}

			void Core::Add (const QString& path, const QUrl& url)
			{
				QFileInfo fi (path);
				if (fi.isDir ())
				{
					QUrl nu = url;
					QDir dir (path);
					QString path = nu.path ();
					path += dir.dirName ();
					if (!path.endsWith ("/"))
						path += "/";
					nu.setPath (path);

					QDir::Filters filters = QDir::Dirs |
						QDir::Files |
						QDir::NoDotAndDotDot;

					if (XmlSettingsManager::Instance ()
							.property ("TransferHiddenFiles").toBool ())
						filters |= QDir::Hidden;
					if (!XmlSettingsManager::Instance ()
							.property ("FollowSymLinks").toBool ())
						filters |= QDir::NoSymLinks;

					QFileInfoList infos = dir.entryInfoList (filters);
					Q_FOREACH (QFileInfo info, infos)
					{
						if (!info.isReadable ())
						{
							qDebug () << Q_FUNC_INFO
								<< "skipping unreadable"
								<< info.absoluteFilePath ();
							continue;
						}

						Add (info.absoluteFilePath (), nu);
					}
				}
				else
				{
					QUrl nu = url;
					QString upath = nu.path ();
					if (!upath.endsWith ("/"))
						upath += "/";
					upath += fi.fileName ();
					nu.setPath (upath);
					TaskData td =
					{
						TaskData::DUpload,
						-1,
						nu,
						path,
						false,
						false
					};
					QueueTask (td);
				}
			}

			void Core::Handle (DownloadEntity e)
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return;

				QUrl url = e.Entity_.toUrl ();

				QFileInfo fi (e.Location_);
				QString dir;
				if (fi.isDir ())
					dir = fi.path ();
				else
					dir = fi.dir ().path ();
	
				TabManager_->AddTab (url, dir);
			}

			int Core::Browse (const QUrl& url)
			{
				TaskData td =
				{
					TaskData::DDownload,
					Proxy_->GetID (),
					url,
					QString (),
					true,
					false
				};
				QueueTask (td, PHigh);
				return td.ID_;
			}

			bool Core::IsAcceptable (int index) const
			{
				return index >= States_.size () ||
					States_.at (index).IsWorking_;
			}

			bool Core::SelectSuitableTask (TaskData *result)
			{
				int limit = XmlSettingsManager::Instance ()
					.property ("WorkersPerDomain").toInt ();
				Q_FOREACH (TaskData td, Tasks_)
				{
					if (td.Paused_)
						continue;

					QString domain = td.URL_.host ();

					if (!WorkersPerDomain_.contains (domain))
						WorkersPerDomain_ [domain] = 0;

					if (WorkersPerDomain_ [domain] >= limit)
						continue;

					WorkersPerDomain_ [domain]++;
					*result = td;
					Tasks_.removeAll (td);
					return true;
				}
				return false;
			}

			void Core::QueueTask (const TaskData& td, Core::Priority p)
			{
				switch (p)
				{
					case PLow:
						beginInsertRows (QModelIndex (),
								Tasks_.size (), Tasks_.size ());
						Tasks_ << td;
						break;
					case PHigh:
						beginInsertRows (QModelIndex (),
								0, 0);
						Tasks_.prepend (td);
						break;
				}
				endInsertRows ();
				SaveTasks ();
				Reschedule ();
			}

			void Core::AddWorker (int i)
			{
				Worker_ptr w (new Worker (i));
				connect (w.get (),
						SIGNAL (error (const QString&, const TaskData&)),
						this,
						SLOT (handleError (const QString&, const TaskData&)));
				connect (w.get (),
						SIGNAL (finished (const TaskData&)),
						this,
						SLOT (handleFinished (const TaskData&)));
				connect (w.get (),
						SIGNAL (fetchedEntry (const FetchedEntry&)),
						this,
						SLOT (handleFetchedEntry (const FetchedEntry&)),
						Qt::QueuedConnection);

				curl_easy_setopt (w->GetHandle ().get (),
						CURLOPT_SHARE, ShareHandle_.get ());

				beginInsertRows (QModelIndex (), i, i);
				Workers_ << w;
				States_ << w->GetState ();
				endInsertRows ();

				Reschedule ();
			}

			void Core::Reschedule ()
			{
				int inQueue = 1;
				do
				{
					CURLMsg *info = 0;
					{
						QMutexLocker l (&MultiHandleMutex_);
						info = curl_multi_info_read (MultiHandle_.get (),
								&inQueue);
					}
					if (!info)
						continue;

					switch (info->msg)
					{
						case CURLMSG_DONE:
							{
								Worker_ptr w = FindWorker (info->easy_handle);
								{
									QMutexLocker l (&MultiHandleMutex_);
									curl_multi_remove_handle (MultiHandle_.get (),
											info->easy_handle);
								}
								w->NotifyFinished (info->data.result);

								if (NumScheduledWorkers_)
								{
									for (int i = 0; i < Workers_.size (); ++i)
									{
										if (Workers_.at (i) != w)
											continue;

										beginRemoveRows (QModelIndex (), i, i);
										Workers_.removeAt (i);
										States_.removeAt (i);
										for (int j = i; j < Workers_.size (); ++j)
											Workers_ [j]->SetID (j - 1);
										--NumScheduledWorkers_;
										endRemoveRows ();
										break;
									}
								}
							}
							break;
						default:
							qWarning () << Q_FUNC_INFO
								<< "unhandled message"
								<< info->msg;
							break;
					}
				}
				while (inQueue);

				while (RunningHandles_ < Workers_.size () &&
						Tasks_.size ())
				{
					TaskData td;
					if (!SelectSuitableTask (&td))
						break;
					Q_FOREACH (Worker_ptr w, Workers_)
						if (!w->IsWorking ())
						{
							CURL_ptr ptr;
							try
							{
								ptr = w->Start (td);
							}
							catch (const QString& msg)
							{
								handleError (msg, td);
								continue;
							}

							{
								QMutexLocker l (&MultiHandleMutex_);
								curl_multi_add_handle (MultiHandle_.get (),
										ptr.get ());
							}
							handlePerform ();
							break;
						}
				}
			}

			Worker_ptr Core::FindWorker (CURL *handle) const
			{
				Q_FOREACH (Worker_ptr w, Workers_)
					if (w->GetHandle ().get () == handle)
						return w;
				qWarning () << Q_FUNC_INFO
					<< "not found handle"
					<< handle;
				return Worker_ptr ();
			}

			void Core::SetupToolbar ()
			{
				ActionPause_ = new QAction (tr ("Pause"),
						Toolbar_);
				ActionPause_->setProperty ("ActionIcon", "lcftp_pause");
				connect (ActionPause_,
						SIGNAL (triggered ()),
						this,
						SLOT (handlePause ()));

				ActionResume_ = new QAction (tr ("Resume"),
						Toolbar_);
				ActionResume_->setProperty ("ActionIcon", "lcftp_resume");
				connect (ActionResume_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleResume ()));

				ActionDelete_ = new QAction (tr ("Delete"),
						Toolbar_);
				ActionDelete_->setProperty ("ActionDelete", "lcftp_delete");
				connect (ActionDelete_,
						SIGNAL (triggered ()),
						this,
						SLOT (handleDelete ()));

				Toolbar_->addAction (ActionPause_);
				Toolbar_->addAction (ActionResume_);
				Toolbar_->addAction (ActionDelete_);
			}

			void Core::SaveTasks ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_LCFTP");
				settings.beginWriteArray ("Tasks");
				settings.remove ("");

				int i = 0;

				Q_FOREACH (Worker_ptr w, Workers_)
				{
					if (!w->IsWorking ())
						continue;

					settings.setArrayIndex (i++);
					settings.setValue ("Task",
							QVariant::fromValue<TaskData> (w->GetTask ()));
				}

				Q_FOREACH (TaskData td, Tasks_)
				{
					settings.setArrayIndex (i++);
					settings.setValue ("Task",
							QVariant::fromValue<TaskData> (td));
				}

				settings.endArray ();
			}

			void Core::loadTasks ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_LCFTP");
				int size = settings.beginReadArray ("Tasks");
				for (int i = 0; i < size; ++i)
				{
					settings.setArrayIndex (i);
					TaskData td = settings.value ("Task").value<TaskData> ();
					QueueTask (td);
				}
				settings.endArray ();
			}

			void Core::handlePerform ()
			{
				bool reschedule = false;
				int prev = RunningHandles_;
				{
					QMutexLocker l (&MultiHandleMutex_);
					while (curl_multi_perform (MultiHandle_.get (),
								&RunningHandles_) == CURLM_CALL_MULTI_PERFORM)
					{
						l.unlock ();
						if (prev != RunningHandles_)
						{
							reschedule = true;
							prev = RunningHandles_;
						}
						l.relock ();
					}
				}
				if (reschedule ||
						prev != RunningHandles_ ||
						(Tasks_.size () &&
						 RunningHandles_ < Workers_.size ()))
					Reschedule ();
			}
			
			void Core::handleError (const QString& msg, const TaskData& td)
			{
				--WorkersPerDomain_ [td.URL_.host ()];

				if (td.ID_ >= 0 &&
						!td.Internal_)
				{
					emit taskError (td.ID_, IDownload::EUnknown);
					emit log (QString ("LCFTP: %1").arg (msg));
				}

				QMessageBox::critical (0,
						tr ("LeechCraft"),
						msg);
			}

			void Core::handleFinished (const TaskData& data)
			{
				--WorkersPerDomain_ [data.URL_.host ()];

				if (data.ID_ >= 0 &&
						!data.Internal_)
				{
					emit downloadFinished (tr ("Download finished: %1")
							.arg (data.Filename_));
					emit taskFinished (data.ID_);
				}

				SaveTasks ();
			}

			void Core::handleFetchedEntry (const FetchedEntry& entry)
			{
				if (entry.PreviousTask_.Internal_)
				{
					emit fetchedEntry (entry);
					return;
				}

				QString name = entry.PreviousTask_.Filename_ + entry.Name_;
				if (entry.IsDir_)
				{
					QDir dir (entry.PreviousTask_.Filename_);
					if (!dir.exists (entry.Name_))
						dir.mkdir (entry.Name_);
					else if (!QFileInfo (name).isDir ())
					{
						QMessageBox::critical (0,
								tr ("LeechCraft"),
								tr ("While mirroring<br />%1<br />to<br />%2<br />"
									"an error occured:<br />%3<br /> already exists.")
									.arg (entry.PreviousTask_.URL_.toString ())
									.arg (QDir::toNativeSeparators (entry.PreviousTask_.Filename_))
									.arg (QDir::toNativeSeparators (name)));
						return;
					}
					name += "/";
				}
				else
					name = CheckName (entry.URL_, entry.PreviousTask_.Filename_);

				TaskData td =
				{
					TaskData::DDownload,
					entry.PreviousTask_.ID_ >= 0 ? Proxy_->GetID () : -1,
					entry.URL_,
					name,
					false,
					false
				};
				QueueTask (td);
			}

			void Core::handleUpdateInterface ()
			{
				States_.clear ();
				for (int i = 0; i < Workers_.size (); ++i)
					States_ << Workers_.at (i)->GetState ();

				emit dataChanged (index (0, 0),
						index (States_.size () - 1, 2));
			}

			void Core::handleTotalNumWorkersChanged ()
			{
				int numWorkers = XmlSettingsManager::Instance ()
					.property ("TotalNumWorkers").toInt ();
				int current = Workers_.size ();
				int delta = numWorkers - current - NumScheduledWorkers_;
				if (delta >= 0)
				{
					NumScheduledWorkers_ = 0;
					for (int i = 0; i < delta; ++i)
						AddWorker (i + current);
				}
				else
					NumScheduledWorkers_ = -delta;
			}

			void Core::handleWorkersPerDomainChanged ()
			{
				Reschedule ();
			}

			void Core::handlePause ()
			{
				QTreeView *tree = Proxy_->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex index = Proxy_->MapToSource (si);
					if (index.model () != GetModel ())
						continue;

					QModelIndex source = WorkersFilter_->mapToSource (index);
					int r = source.row ();

					int working = Workers_.size ();

					if (r < working)
						Workers_.at (r)->Pause ();
					else
						Tasks_ [r - working].Paused_ = true;
				}
			}

			void Core::handleResume ()
			{
				QTreeView *tree = Proxy_->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex index = Proxy_->MapToSource (si);
					if (index.model () != GetModel ())
						continue;

					QModelIndex source = WorkersFilter_->mapToSource (index);
					int r = source.row ();

					int working = Workers_.size ();

					if (r < working)
						Workers_.at (r)->Resume ();
					else
						Tasks_ [r - working].Paused_ = false;
				}

				Reschedule ();
			}

			void Core::handleDelete ()
			{
				QTreeView *tree = Proxy_->GetCurrentView ();
				if (!tree)
					return;

				QItemSelectionModel *sel = tree->selectionModel ();
				if (!sel)
					return;

				QModelIndexList sis = sel->selectedRows ();
				QList<int> tasksIndexes;
				Q_FOREACH (QModelIndex si, sis)
				{
					QModelIndex index = Proxy_->MapToSource (si);
					if (index.model () != GetModel ())
						continue;

					QModelIndex source = WorkersFilter_->mapToSource (index);
					int r = source.row ();

					int working = Workers_.size ();

					if (r < working)
					{
						{
							QMutexLocker l (&MultiHandleMutex_);
							curl_multi_remove_handle (MultiHandle_.get (),
									Workers_.at (r)->GetHandle ().get ());
						}
						Workers_.at (r)->Abort ();
					}
					else
						tasksIndexes << (r - working);
				}

				std::sort (tasksIndexes.begin (), tasksIndexes.end (),
						std::greater<int> ());

				Q_FOREACH (int index, tasksIndexes)
				{
					beginRemoveRows (QModelIndex (), index, index);
					Tasks_.removeAt (index);
					endRemoveRows ();
				}

				Reschedule ();

				SaveTasks ();
			}
		};
	};
};

