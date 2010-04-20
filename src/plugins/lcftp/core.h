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

#ifndef PLUGINS_LCFTP_CORE_H
#define PLUGINS_LCFTP_CORE_H
#include <boost/scoped_array.hpp>
#include <QAbstractItemModel>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QMutex>
#include <QList>
#include <curl/curl.h>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/structures.h>
#include <plugininterface/guarded.h>
#include "worker.h"

class QToolBar;
class QAction;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class InactiveWorkersFilter;
			class WatchThread;
			class TabManager;
			class SummaryTab;

			typedef boost::shared_ptr<CURLM> CURLM_ptr;
			typedef boost::shared_ptr<CURLSH> CURLSH_ptr;

			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				friend class WatchThread;

				static Core *Instance_;
				static QMutex InstanceMutex_;

				struct CurlGlobalGuard
				{
					CurlGlobalGuard ()
					{
						curl_global_init (CURL_GLOBAL_ALL);
					}

					~CurlGlobalGuard ()
					{
						curl_global_cleanup ();
					}
				};

				WatchThread *WatchThread_;

				CurlGlobalGuard Guard_;
				QMutex MultiHandleMutex_;
				CURLM_ptr MultiHandle_;
				CURLSH_ptr ShareHandle_;

				QList<TaskData> Tasks_;

				ICoreProxy_ptr Proxy_;

				QList<Worker_ptr> Workers_;
				bool Quitting_;
				QList<Worker::TaskState> States_;
				QMap<QString, int> WorkersPerDomain_;
				TabManager *TabManager_;

				boost::shared_ptr<InactiveWorkersFilter> WorkersFilter_;

				int NumScheduledWorkers_;
				int RunningHandles_;

				QToolBar *Toolbar_;
				QAction *ActionPause_,
						*ActionResume_,
						*ActionDelete_;

				SummaryTab *SummaryTab_;

				enum Priority
				{
					PLow,
					PHigh
				};

				Core ();
			public:
				enum
				{
					RoleDownSpeedLimit = 200,
					RoleUpSpeedLimit,
					RoleLog
				};
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetCoreProxy () const;

				QAbstractItemModel* GetModel () const;
				qint64 GetDownloadSpeed () const;
				qint64 GetUploadSpeed () const;
				TabManager* GetTabManager () const;

				int columnCount (const QModelIndex& = QModelIndex ()) const;
				QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex& = QModelIndex ()) const;
				bool setData (const QModelIndex&, const QVariant&, int);

				QStringList Provides () const;
				bool IsOK (const DownloadEntity&) const;
				/** Downloads given download entity.
				 */
				int Add (DownloadEntity);
				/** Downloads the given url to the given path. Whether
				 * the target files are checked to is controlled by
				 * check.
				 */
				int Add (const QUrl& url, const QString& path, bool check);
				/** Uploads the given path to the given url.
				 */
				void Add (const QString& path, const QUrl& url);
				void Handle (DownloadEntity);
				int Browse (const QUrl&);

				bool IsAcceptable (int) const;
				bool SelectSuitableTask (TaskData*);
			private:
				void QueueTask (const TaskData&, Priority = PLow);
				void AddWorker (int);
				void Reschedule ();
				Worker_ptr FindWorker (CURL*) const;
				void SetupToolbar ();
				void SaveTasks ();
			public slots:
				void loadTasks ();
				void handleError (const QString&, const TaskData&);
				void handleFinished (const TaskData&);
				void handleFetchedEntry (const FetchedEntry&);
			private slots:
				void handlePerform ();
				void handleUpdateInterface ();
				void handleTotalNumWorkersChanged ();
				void handleWorkersPerDomainChanged ();
				void handlePause ();
				void handleResume ();
				void handleDelete ();
			signals:
				void taskFinished (int);
				void taskRemoved (int);
				void taskError (int, IDownload::Error);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void fetchedEntry (const FetchedEntry&);
			};
		};
	};
};

#endif

