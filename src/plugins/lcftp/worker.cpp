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
			, Exit_ (false)
			{
			}

			Worker::~Worker ()
			{
			}

			void Worker::SetExit ()
			{
				Exit_ = true;
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

				bool first = true;

				while (true)
				{
					if (!first)
						Core::Instance ().FinishedTask ();
					first = false;

					TaskData td = Core::Instance ().GetNextTask ();
					if (Exit_)
					{
						File_.reset ();
						break;
					}

					File_.reset (new QFile (td.Filename_));
					if (!File_->open (QIODevice::WriteOnly))
					{
						emit error (tr ("Could not open file<br />%1<br />%2")
								.arg (td.Filename_)
								.arg (File_->errorString ()));
						continue;
					}
					curl_easy_setopt (handle.get (),
							CURLOPT_URL, td.URL_.toEncoded ().constData ());
					curl_easy_setopt (handle.get (),
							CURLOPT_WRITEFUNCTION, write_data);
					curl_easy_setopt (handle.get (),
							CURLOPT_WRITEDATA, w.get ());

					CURLcode result = curl_easy_perform (handle.get ());
					if (result)
					{
						QString errstr (curl_easy_strerror (result));
						qWarning () << Q_FUNC_INFO
							<< result
							<< errstr;
						emit error (errstr);
					}
					File_->flush ();
				}
				qDebug () << "quitting worker";
			}

			size_t Worker::WriteData (void *buffer, size_t size, size_t nmemb)
			{
				const char *start = static_cast<char*> (buffer);
				size_t written = File_->write (start, size * nmemb);
				return written;
			}
		};
	};
};

