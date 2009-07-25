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
#include "worker.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class InactiveWorkersFilter;

			typedef boost::shared_ptr<CURLM> CURLM_ptr;
			typedef boost::shared_ptr<CURLSH> CURLSH_ptr;

			class Core : public QAbstractItemModel
			{
				Q_OBJECT

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

				CurlGlobalGuard Guard_;
				CURLM_ptr MultiHandle_;
				CURLSH_ptr ShareHandle_;

				QList<TaskData> Tasks_;

				ICoreProxy_ptr Proxy_;

				QList<Worker_ptr> Workers_;
				bool Quitting_;
				QList<Worker::TaskState> States_;
				QMap<QString, int> WorkersPerDomain_;

				boost::shared_ptr<InactiveWorkersFilter> WorkersFilter_;

				int NumScheduledWorkers_;
				int RunningHandles_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);

				QAbstractItemModel* GetModel () const;
				qint64 GetDownloadSpeed () const;
				qint64 GetUploadSpeed () const;

				int columnCount (const QModelIndex& = QModelIndex ()) const;
				QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex& = QModelIndex ()) const;

				QStringList Provides () const;
				bool IsOK (const DownloadEntity&) const;
				int Add (DownloadEntity);

				bool IsAcceptable (int) const;
				bool SelectSuitableTask (TaskData*);
			private:
				void QueueTask (const TaskData&);
				void AddWorker (int);
				void Reschedule ();
				Worker_ptr FindWorker (CURL*) const;
			public slots:
				void handleError (const QString&, const TaskData&);
				void handleFinished (const TaskData&);
				void handleFetchedEntry (const FetchedEntry&);
			private slots:
				void handlePerform ();
				void handleUpdateInterface ();
				void handleThreadFinished ();
				void handleTotalNumWorkersChanged ();
				void handleWorkersPerDomainChanged ();
			signals:
				void taskFinished (int);
				void taskRemoved (int);
				void taskError (int, IDownload::Error);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void downloadFinished (const QString&);
			};
		};
	};
};

#endif

