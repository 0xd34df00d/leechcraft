#ifndef PLUGINS_LCFTP_CORE_H
#define PLUGINS_LCFTP_CORE_H
#include <boost/scoped_array.hpp>
#include <QAbstractItemModel>
#include <QWaitCondition>
#include <QReadWriteLock>
#include <QMutex>
#include <QList>
#include <interfaces/iinfo.h>
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
				void FinishedTask (int = -1);
			private:
				void QueueTask (const TaskData&);
			public slots:
				void handleError (const QString&);
			private slots:
				void handleUpdateInterface ();
			};
		};
	};
};

#endif

