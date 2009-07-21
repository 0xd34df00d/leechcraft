#ifndef PLUGINS_LCFTP_WORKER_H
#define PLUGINS_LCFTP_WORKER_H
#include <boost/shared_ptr.hpp>
#include <QThread>
#include <QFile>
#include <QUrl>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct Wrapper;

			struct TaskData
			{
				QString Filename_;
				QUrl URL_;
			};

			class Worker : public QThread
			{
				Q_OBJECT

				friend struct Wrapper;
				boost::shared_ptr<QFile> File_;
			public:
				Worker (QObject* = 0);
				virtual ~Worker ();
			protected:
				void run ();
			private:
				size_t WriteData (void*, size_t, size_t);
			signals:
				void error (const QString&);
			};
		};
	};
};

#endif

