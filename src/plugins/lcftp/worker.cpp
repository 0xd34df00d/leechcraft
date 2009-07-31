#include "worker.h"
#include <curl/curl.h>
#include "core.h"
#include "xmlsettingsmanager.h"

extern "C"
{
#include "3dparty/ftpparse.h"
};

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

				size_t ReadData (char *buf, size_t size, size_t nmemb)
				{
					return W_->ReadData (buf, size, nmemb);
				}

				size_t ListDir (void *buf, size_t size, size_t nmemb)
				{
					return W_->ListDir (buf, size, nmemb);
				}

				int Progress (double dlt, double dln, double ult, double uln)
				{
					return W_->Progress (dlt, dln, ult, uln);
				}
			};

			namespace
			{
				size_t write_data (void *buf, size_t size, size_t nmemb, void *userp)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->WriteData (buf, size, nmemb);
				}

				size_t read_data (char *buf, size_t size, size_t nmemb, void *userp)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->ReadData (buf, size, nmemb);
				}

				size_t list_dir (void *buf, size_t size, size_t nmemb, void *userp)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->ListDir (buf, size, nmemb);
				}
				
				int progress_function (void *userp,
						double dltotal, double dlnow,
						double ultotal, double ulnow)
				{
					Wrapper *w = static_cast<Wrapper*> (userp);
					return w->Progress (dltotal, dlnow, ultotal, ulnow);
				}
			};

			Worker::Worker (int id, QObject *parent)
			: QObject (parent)
			, ID_ (id)
			, Handle_ (curl_easy_init (), curl_easy_cleanup)
			, W_ (new Wrapper (this))
			, IsWorking_ (false)
			, Paused_ (false)
			, DLNow_ (0)
			, DLTotal_ (0)
			, ULNow_ (0)
			, ULTotal_ (0)
			, InitialSize_ (0)
			{
				curl_easy_setopt (Handle_.get (),
						CURLOPT_WRITEDATA, W_.get ());
				curl_easy_setopt (Handle_.get (),
						CURLOPT_READFUNCTION, read_data);
				curl_easy_setopt (Handle_.get (),
						CURLOPT_READDATA, W_.get ());
				curl_easy_setopt (Handle_.get (),
						CURLOPT_NOPROGRESS, 0L);
				curl_easy_setopt (Handle_.get (),
						CURLOPT_PROGRESSFUNCTION, progress_function);
				curl_easy_setopt (Handle_.get (),
						CURLOPT_PROGRESSDATA, W_.get ());
				curl_easy_setopt (Handle_.get (),
						CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

				Reset ();
			}

			Worker::~Worker ()
			{
				qDebug () << Q_FUNC_INFO;
			}

			bool Worker::IsWorking () const
			{
				return IsWorking_;
			}

			void Worker::SetID (int id)
			{
				ID_ = id;
			}

			Worker::TaskState Worker::GetState () const
			{
				int secs = StartDT_.secsTo (QDateTime::currentDateTime ());
				quint64 dl = secs ? DLNow_ / secs : 0;
				quint64 ul = secs ? ULNow_ / secs : 0;

				quint64 is = InitialSize_;

				TaskState result =
				{
					ID_,
					IsWorking_,
					Task_.URL_,
					qMakePair<quint64, quint64> (DLNow_ + is, DLTotal_ + is),
					qMakePair<quint64, quint64> (ULNow_ + is, ULTotal_ + is),
					dl,
					ul,
					Task_.Direction_,
					Paused_
				};
				return result;
			}

			QUrl Worker::GetURL () const
			{
				return Task_.URL_;
			}

			CURL_ptr Worker::GetHandle () const
			{
				return Handle_;
			}

			CURL_ptr Worker::Start (const TaskData& td)
			{
				IsWorking_ = true;
				UpdateHandleSettings (Handle_);
				StartDT_ = QDateTime::currentDateTime ();
				Task_ = td;

				HandleTask (td, Handle_);
				return Handle_;
			}

			void Worker::NotifyFinished (CURLcode result)
			{
				if (result)
				{
					QString errstr (curl_easy_strerror (result));
					qWarning () << Q_FUNC_INFO
						<< result
						<< errstr;
					emit error (errstr, Task_);
				}

				if (File_)
					File_->close ();
				else
					ParseBuffer (Task_);
				emit finished (Task_);

				IsWorking_ = false;
			}

			void Worker::Abort ()
			{
				if (File_)
					File_->close ();
				emit error (tr ("Aborted"), Task_);

				IsWorking_ = false;
			}

			void Worker::Pause ()
			{
				curl_easy_pause (Handle_.get (), CURLPAUSE_ALL);
				Paused_ = true;
			}

			void Worker::Resume ()
			{
				curl_easy_pause (Handle_.get (), CURLPAUSE_CONT);
				Paused_ = false;
			}

			/** Sets up the libcurl handle to perform the task and
			 * starts the task.
			 */
			void Worker::HandleTask (const TaskData& td, CURL_ptr handle)
			{
				curl_easy_setopt (handle.get (),
						CURLOPT_URL, td.URL_.toEncoded ().constData ());
				curl_easy_setopt (handle.get (),
						CURLOPT_DIRLISTONLY, 0L);

				if (td.Direction_ == TaskData::DDownload)
				{
					curl_easy_setopt (handle.get (),
							CURLOPT_UPLOAD, 0L);

					if (td.URL_.toString ().endsWith ("/") ||
							td.Filename_.isNull ())
					{
						if (td.Filename_.isNull () &&
								!td.URL_.toString ().endsWith ("/"))
							curl_easy_setopt (handle.get (),
									CURLOPT_DIRLISTONLY, 1L);

						curl_easy_setopt (handle.get (),
								CURLOPT_WRITEFUNCTION, list_dir);

						File_.reset ();
						ListBuffer_.reset (new QBuffer ());
						curl_easy_setopt (handle.get (),
								CURLOPT_RESUME_FROM_LARGE, 0L);
					}
					else
					{
						curl_easy_setopt (handle.get (),
								CURLOPT_WRITEFUNCTION, write_data);

						ListBuffer_.reset ();
						File_.reset (new QFile (td.Filename_));
						if (!File_->open (QIODevice::WriteOnly | QIODevice::Append) &&
								!File_->open (QIODevice::WriteOnly))
							throw (tr ("Could not open file<br />%1<br />%2")
										.arg (td.Filename_)
										.arg (File_->errorString ()));

						InitialSize_ = File_->size ();
						curl_easy_setopt (handle.get (),
								CURLOPT_RESUME_FROM_LARGE, File_->size ());
					}
				}
				else if (td.Direction_ == TaskData::DUpload)
				{
					curl_easy_setopt (handle.get (),
							CURLOPT_UPLOAD, 1L);

					ListBuffer_.reset ();
					File_.reset (new QFile (td.Filename_));
					if (!File_->open (QIODevice::ReadOnly))
						throw (tr ("Could not open file<br />%1<br />%2")
								.arg (td.Filename_)
								.arg (File_->errorString ()));

					qint64 filesize = File_->size ();
					curl_easy_setopt (handle.get (),
							CURLOPT_INFILESIZE_LARGE, filesize);
				}
			}

			/** Parses the buffer in case the task referred to a
			 * directory, and we got a listing instead of file.
			 *
			 * Throws found entries to the Core.
			 */
			void Worker::ParseBuffer (const TaskData& td)
			{
				QByteArray buf = ListBuffer_->buffer ();
				QList<QByteArray> bstrs = buf.split ('\n');
				QStringList result;
				Q_FOREACH (QByteArray bstr, bstrs)
				{
					struct ftpparse fp;
					if (!ftpparse (&fp, bstr.data (), bstr.size ()))
					{
						qWarning () << Q_FUNC_INFO
							<< "unable to parse"
							<< bstr;
						continue;
					}

					QString name = QString (QByteArray (fp.name, fp.namelen));
					if (!fp.flagtrycwd && !fp.flagtryretr)
					{
						qWarning () << Q_FUNC_INFO
							<< "skipping"
							<< name;
						continue;
					}

					QUrl itemUrl = Task_.URL_;
					itemUrl.setPath (itemUrl.path () + name);
					if (fp.flagtrycwd)
						itemUrl.setPath (itemUrl.path () + "/");

					QDateTime dt;
					if (fp.mtimetype != FTPPARSE_MTIME_UNKNOWN)
						dt.setTime_t (fp.mtime);

					FetchedEntry fe =
					{
						itemUrl,
						fp.size,
						dt,
						fp.flagtrycwd,
						name,
						td
					};

					emit fetchedEntry (fe);
				}
			}

			void Worker::Reset ()
			{
				DLNow_ = 0;
				DLTotal_ = 0;
				ULNow_ = 0;
				ULTotal_ = 0;
				IsWorking_ = false;
				Task_ = TaskData ();
			}

			size_t Worker::WriteData (void *buffer, size_t size, size_t nmemb)
			{
				const char *start = static_cast<char*> (buffer);
				size_t written = File_->write (start, size * nmemb);
				return written;
			}

			size_t Worker::ReadData (char *buffer, size_t size, size_t nmemb)
			{
				return File_->read (buffer, size * nmemb);
			}
			
			size_t Worker::ListDir (void *buffer, size_t size, size_t nmemb)
			{
				const char *start = static_cast<char*> (buffer);
				size_t result = size * nmemb;
				ListBuffer_->buffer ().append (start, result);
				return result;
			}
			
			int Worker::Progress (double dlt, double dln, double ult, double uln)
			{
				DLNow_ = dln;
				DLTotal_ = dlt;
				ULNow_ = uln;
				ULTotal_ = ult;
				return 0;
			}

			void Worker::UpdateHandleSettings (CURL_ptr handle)
			{
				/** Ports
				 */
				if (XmlSettingsManager::Instance ()
						.property ("CustomPortRange").toBool ())
				{
					QList<QVariant> ports = XmlSettingsManager::Instance ().property ("TCPPortRange").toList ();
					curl_easy_setopt (handle.get (),
							CURLOPT_LOCALPORT, ports.at (0).toInt ());
					curl_easy_setopt (handle.get (),
							CURLOPT_LOCALPORTRANGE, ports.at (1).toInt () - ports.at (0).toInt () + 1);
				}
				else
				{
					curl_easy_setopt (handle.get (),
							CURLOPT_LOCALPORT, 0L);
					curl_easy_setopt (handle.get (),
							CURLOPT_LOCALPORTRANGE, 0L);
				}

				/** Proxy stuff
				 */
				if (XmlSettingsManager::Instance ()
						.property ("ProxyEnabled").toBool ())
				{
					QString str = QString ("%1:%2")
						.arg (XmlSettingsManager::Instance ()
								.property ("ProxyHost").toString ())
						.arg (XmlSettingsManager::Instance ()
								.property ("ProxyPort").toInt ());
					curl_easy_setopt (handle.get (),
							CURLOPT_PROXY, str.toStdString ().c_str ());

					QString type = XmlSettingsManager::Instance ()
						.property ("ProxyType").toString ();
					if (type == "http")
						curl_easy_setopt (handle.get (),
								CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
					else if (type == "http10")
						curl_easy_setopt (handle.get (),
								CURLOPT_PROXYTYPE, CURLPROXY_HTTP_1_0);
					else if (type == "socks4")
						curl_easy_setopt (handle.get (),
								CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
					else if (type == "socks4a")
						curl_easy_setopt (handle.get (),
								CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
					else if (type == "socks5")
						curl_easy_setopt (handle.get (),
								CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
					else if (type == "socks5-hostname")
						curl_easy_setopt (handle.get (),
								CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);

					QString creds = QString ("%1:%2")
						.arg (XmlSettingsManager::Instance ()
								.property ("ProxyLogin").toString ())
						.arg (XmlSettingsManager::Instance ()
								.property ("ProxyPassword").toString ());
					curl_easy_setopt (handle.get (),
							CURLOPT_PROXYUSERPWD, creds.toStdString ().c_str ());

					curl_easy_setopt (handle.get (),
							CURLOPT_NOPROXY, XmlSettingsManager::Instance ()
								.property ("NoProxy").toString ().toStdString ().c_str ());

					curl_easy_setopt (handle.get (),
							CURLOPT_HTTPPROXYTUNNEL, XmlSettingsManager::Instance ()
								.property ("ProxyTunnel").toBool () ? 1L : 0L);
				}
				else
					curl_easy_setopt (handle.get (),
							CURLOPT_PROXY, "");

				/** EPRT
				 */
				curl_easy_setopt (handle.get (),
						CURLOPT_FTP_USE_EPRT, XmlSettingsManager::Instance ()
							.property ("UseEPRT").toBool () ? 1L : 0L);

				/** EPSV
				 */
				curl_easy_setopt (handle.get (),
						CURLOPT_FTP_USE_EPRT, XmlSettingsManager::Instance ()
							.property ("UseEPSV").toBool () ? 1L : 0L);

				/** Ignore PASV IP
				 */
				curl_easy_setopt (handle.get (),
						CURLOPT_FTP_SKIP_PASV_IP, XmlSettingsManager::Instance ()
							.property ("SkipPasvIP").toBool () ? 1L : 0L);
			}
		};
	};
};

