#include "core.h"
#include <QUrl>
#include <QMutexLocker>
#include <curl/curl.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Core* Core::Instance_ = 0;
			QMutex Core::InstanceMutex_;

			Core::Core ()
			{
				curl_global_init (CURL_GLOBAL_ALL);
			}

			Core& Core::Instance ()
			{
				if (!Instance_)
				{
					QMutexLocker locker (&InstanceMutex_);
					if (!Instance_)
						Instance_ = new Core ();
				}
				return *Instance_;
			}

			void Core::Release ()
			{
				curl_global_cleanup ();
				deleteLater ();
			}

			QStringList Core::Provides () const
			{
				curl_version_info_data *data = curl_version_info (CURLVERSION_NOW);
				QStringList result ("ftp");
				if (data->age > 0)
				{
					if (data->features & CURL_VERSION_SSL)
						result << "ftps";
				}
				return result;
			}

			bool Core::IsOK (const DownloadEntity& e) const
			{
				if (!e.Entity_.canConvert<QUrl> ())
					return false;

				QUrl url = e.Entity_.toUrl ();
				if (Provides ().contains (url.scheme ()))
					return true;
				else
					return false;
			}

			int Core::Add (DownloadEntity e)
			{
			}

			TaskData Core::GetNextTask ()
			{
				WorkerWaitMutex_.lock ();
				WorkerWait_.wait (&WorkerWaitMutex_);
				return Tasks_.Val ().takeFirst ();
			}

			void Core::FinishedTask ()
			{
				WorkerWaitMutex_.unlock ();
			}
		};
	};
};

