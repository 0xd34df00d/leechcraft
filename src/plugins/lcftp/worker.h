#ifndef PLUGINS_LCFTP_WORKER_H
#define PLUGINS_LCFTP_WORKER_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QFile>
#include <QBuffer>
#include <QUrl>
#include <QDateTime>
#include <curl/curl.h>
#include "structures.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct Wrapper;
			typedef boost::shared_ptr<Wrapper> Wrapper_ptr;

			typedef boost::shared_ptr<CURL> CURL_ptr;

			class Worker : public QObject
			{
				Q_OBJECT

				friend struct Wrapper;
				int ID_;
				CURL_ptr Handle_;
				Wrapper_ptr W_;
				boost::shared_ptr<QFile> File_;
				boost::shared_ptr<QBuffer> ListBuffer_;
				bool IsWorking_;
				quint64 DLNow_,
					DLTotal_,
					ULNow_,
					ULTotal_,
					InitialSize_;
				QDateTime StartDT_;
				TaskData Task_;
			public:
				struct TaskState
				{
					int WorkID_;
					bool IsWorking_;
					QUrl URL_;
					QPair<quint64, quint64> DL_;
					QPair<quint64, quint64> UL_;
					quint64 DLSpeed_;
					quint64 ULSpeed_;
				};

				Worker (int, QObject* = 0);
				virtual ~Worker ();

				bool IsWorking () const;
				void SetID (int);
				TaskState GetState () const;
				QUrl GetURL () const;
				CURL_ptr GetHandle () const;
				/** Prepares to perform the given task, starts it and
				 * returns immediately.
				 * 
				 * @param[in] task The task description.
				 */
				CURL_ptr Start (const TaskData& task);
				/** This function is used by the Core to notify this
				 * worker that it has finished and now should do some
				 * post-processing, if any.
				 *
				 * @param[in] result The result of the operation.
				 */
				void NotifyFinished (CURLcode result);
			private:
				void HandleTask (const TaskData&, CURL_ptr);
				void ParseBuffer (const TaskData&);
				void Reset ();
				size_t WriteData (void*, size_t, size_t);
				size_t ReadData (char*, size_t, size_t);
				size_t ListDir (void*, size_t, size_t);
				int Progress (double, double, double, double);
				void UpdateHandleSettings (CURL_ptr);
			signals:
				void error (const QString&, const TaskData&);
				void finished (const TaskData&);
				void fetchedEntry (const FetchedEntry&);
			};

			typedef boost::shared_ptr<Worker> Worker_ptr;
		};
	};
};

#endif

