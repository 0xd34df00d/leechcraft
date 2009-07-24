#ifndef PLUGINS_LCFTP_CORE_H
#define PLUGINS_LCFTP_CORE_H
#include <boost/scoped_array.hpp>
#include <QAbstractItemModel>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QMutex>
#include <QList>
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

			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				static Core *Instance_;
				static QMutex InstanceMutex_;

				QWaitCondition WorkerWait_;
				QMutex WorkerWaitMutex_;

				QList<TaskData> Tasks_;
				QReadWriteLock TasksLock_;

				ICoreProxy_ptr Proxy_;

				QList<Worker_ptr> Workers_;
				Util::Guarded<bool> Quitting_;
				QList<Worker::TaskState> States_;

				boost::shared_ptr<InactiveWorkersFilter> WorkersFilter_;

				Util::Guarded<QList<Worker*> > ScheduledWorkers_;
				int NumScheduledWorkers_;

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

				// These are called from workers' threads.
				TaskData GetNextTask ();
				void FinishedTask (Worker*, int = -1);
			private:
				void QueueTask (const TaskData&);
				void AddWorker (int);
			public slots:
				void handleError (const QString&, const TaskData&);
				void handleFinished (const TaskData&);
				void handleFetchedEntry (const FetchedEntry&);
			private slots:
				void handleUpdateInterface ();
				void handleScheduledRemoval ();
				void handleThreadFinished ();
				void handleTotalNumWorkersChanged ();
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

