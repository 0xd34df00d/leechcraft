#ifndef PLUGINS_LCFTP_CORE_H
#define PLUGINS_LCFTP_CORE_H
#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <QList>
#include <interfaces/structures.h>
#include <plugininterface/guarded.h>
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

				Util::Guarded<QList<TaskData> > Tasks_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();

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

