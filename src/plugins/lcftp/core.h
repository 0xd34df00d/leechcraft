#ifndef PLUGINS_LCFTP_CORE_H
#define PLUGINS_LCFTP_CORE_H
#include <boost/scoped_array.hpp>
#include <QObject>
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
			class Core : public QObject
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

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);

				QStringList Provides () const;
				bool IsOK (const DownloadEntity&) const;
				int Add (DownloadEntity);

				// These are called from workers' threads.
				TaskData GetNextTask ();
				void FinishedTask ();
			};
		};
	};
};

#endif

