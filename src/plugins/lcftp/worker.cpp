#include "worker.h"
#include <curl/curl.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct Wrapper
			{
				Worker *W_;

				Wrapper (Worker *w)
				: W_ (w)
				{
				}

				size_t WriteData (void *buf, size_t size, size_t nmemb)
				{
					return W_->WriteData (buf, size, nmemb);
				}

				int Progress (double dlt, double dln, double ult, double uln)
				{
					return W_->Progress (dlt, dln, ult, uln);
				}
			};

			Worker::Worker (int id, QObject *parent)
			: QThread (parent)
			, ID_ (id)
			, Exit_ (false)
			{
				Reset ();
			}

			Worker::~Worker ()
			{
			}

			bool Worker::IsWorking () const
			{
				return IsWorking_;
			}

			void Worker::SetExit ()
			{
				Exit_ = true;
			}

			Worker::TaskState Worker::GetState () const
			{
				TaskState result =
				{
					ID_,
					IsWorking_,
					URL_,
					qMakePair<quint64, quint64> (DLNow_, DLTotal_),
					qMakePair<quint64, quint64> (ULNow_, ULTotal_)
				};
				return result;
			}

			QPair<quint64, quint64> Worker::GetDL () const
			{
				return qMakePair<quint64, quint64> (DLNow_, DLTotal_);
			}

			QPair<quint64, quint64> Worker::GetUL () const
			{
				return qMakePair<quint64, quint64> (ULNow_, ULTotal_);
			}

			QUrl Worker::GetURL () const
			{
				return URL_;
			}

			namespace
			{
				size_t write_data (void *buf, size_t size, size_t nmemb, void *userp)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->WriteData (buf, size, nmemb);
				}
				
				int progress_function (void *userp,
						double dltotal, double dlnow,
						double ultotal, double ulnow)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->Progress (dltotal, dlnow, ultotal, ulnow);
				}
			};

			void Worker::run ()
			{
				CURL_ptr handle (curl_easy_init (),
						curl_easy_cleanup);
				boost::shared_ptr<Wrapper> w (new Wrapper (this));

				curl_easy_setopt (handle.get (),
						CURLOPT_WRITEFUNCTION, write_data);
				curl_easy_setopt (handle.get (),
						CURLOPT_WRITEDATA, w.get ());
				curl_easy_setopt (handle.get (),
						CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt (handle.get (),
						CURLOPT_PROGRESSFUNCTION, progress_function);
				curl_easy_setopt (handle.get (),
						CURLOPT_PROGRESSDATA, w.get ());

				int id = -1;

				while (true)
				{
					if (id >= 0)
						Core::Instance ().FinishedTask (id);

					TaskData td = Core::Instance ().GetNextTask ();
					if (Exit_)
					{
						Core::Instance ().FinishedTask ();
						File_.reset ();
						break;
					}

					id = td.ID_;

					URL_ = td.URL_;

					IsWorking_ = true;

					HandleTask (td, handle);
					Reset ();
				}
			}

			void Worker::HandleTask (const TaskData& td, CURL_ptr handle)
			{
				File_.reset (new QFile (td.Filename_));
				if (!File_->open (QIODevice::WriteOnly))
				{
					emit error (tr ("Could not open file<br />%1<br />%2")
							.arg (td.Filename_)
							.arg (File_->errorString ()));
					return;
				}
				curl_easy_setopt (handle.get (),
						CURLOPT_URL, td.URL_.toEncoded ().constData ());

				CURLcode result = curl_easy_perform (handle.get ());
				File_->flush ();
				if (result)
				{
					QString errstr (curl_easy_strerror (result));
					qWarning () << Q_FUNC_INFO
						<< result
						<< errstr;
					emit error (errstr);
				}
			}

			void Worker::Reset ()
			{
				DLNow_ = 0;
				DLTotal_ = 0;
				ULNow_ = 0;
				ULTotal_ = 0;
				IsWorking_ = false;
				URL_ = QUrl ();
			}

			size_t Worker::WriteData (void *buffer, size_t size, size_t nmemb)
			{
				const char *start = static_cast<char*> (buffer);
				size_t written = File_->write (start, size * nmemb);
				return written;
			}
			
			int Worker::Progress (double dlt, double dln, double ult, double uln)
			{
				DLNow_ = dln;
				DLTotal_ = dlt;
				ULNow_ = uln;
				ULTotal_ = ult;
				return 0;
			}
		};
	};
};

