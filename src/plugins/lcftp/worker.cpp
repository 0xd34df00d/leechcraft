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
			};

			Worker::Worker (QObject *parent)
			: QThread (parent)
			{
			}

			Worker::~Worker ()
			{
			}

			namespace
			{
				size_t write_data (void *buf, size_t size, size_t nmemb, void *userp)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->WriteData (buf, size, nmemb);
				}
			};

			void Worker::run ()
			{
				boost::shared_ptr<CURL> handle (curl_easy_init (),
						curl_easy_cleanup);
				boost::shared_ptr<Wrapper> w (new Wrapper (this));

				while (true)
				{
					TaskData td = Core::Instance ().GetNextTask ();
					curl_easy_setopt (handle.get (),
							CURLOPT_URL, td.URL_.toEncoded ().constData ());
					curl_easy_setopt (handle.get (),
							CURLOPT_WRITEFUNCTION, write_data);
					curl_easy_setopt (handle.get (),
							CURLOPT_WRITEDATA, w.get ());

					CURLcode result = curl_easy_perform (handle.get ());
					if (!result)
					{
						QString errstr (curl_easy_strerror (result));
						qWarning () << Q_FUNC_INFO
							<< result
							<< errstr;
						emit error (errstr);
					}
					Core::Instance ().FinishedTask ();
				}
			}

			size_t Worker::WriteData (void *buffer, size_t size, size_t nmemb)
			{
				const char *start = static_cast<char*> (buffer);
				size_t written = File_->write (start, size * nmemb);
				qDebug () << Q_FUNC_INFO << written << size * nmemb;
				return written;
			}
		};
	};
};

