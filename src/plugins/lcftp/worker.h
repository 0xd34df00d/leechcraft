#ifndef PLUGINS_LCFTP_WORKER_H
#define PLUGINS_LCFTP_WORKER_H
#include <boost/shared_ptr.hpp>
#include <QThread>
#include <QFile>
#include <QUrl>
#include <plugininterface/guarded.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct Wrapper;

			struct TaskData
			{
				int ID_;
				QUrl URL_;
				QString Filename_;
			};

			class Worker : public QThread
			{
				Q_OBJECT

				friend struct Wrapper;
				boost::shared_ptr<QFile> File_;
				Util::Guarded<bool> Exit_;
			public:
				Worker (QObject* = 0);
				virtual ~Worker ();

				void SetExit ();
			protected:
				void run ();
			private:
				size_t WriteData (void*, size_t, size_t);
			signals:
				void error (const QString&);
			};

			typedef boost::shared_ptr<Worker> Worker_ptr;
		};
	};
};

#endif

