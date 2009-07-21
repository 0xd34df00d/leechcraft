#ifndef PLUGINS_LCFTP_WORKER_H
#define PLUGINS_LCFTP_WORKER_H
#include <boost/shared_ptr.hpp>
#include <QThread>
#include <QFile>
#include <QUrl>
#include <curl/curl.h>
#include <plugininterface/guarded.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			typedef boost::shared_ptr<CURL> CURL_ptr;

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
				Util::Guarded<quint64> DLNow_,
					DLTotal_,
					ULNow_,
					ULTotal_;
			public:
				Worker (QObject* = 0);
				virtual ~Worker ();

				void SetExit ();
				QPair<quint64, quint64> GetDL () const;
				QPair<quint64, quint64> GetUL () const;
			protected:
				void run ();
			private:
				void HandleTask (const TaskData&, CURL_ptr);
				void Reset ();
				size_t WriteData (void*, size_t, size_t);
				int Progress (double, double, double, double);
			signals:
				void error (const QString&);
			};

			typedef boost::shared_ptr<Worker> Worker_ptr;
		};
	};
};

#endif

