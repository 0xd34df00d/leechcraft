#include "core.h"
#include <QUrl>
#include <curl/curl.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			Core::Core ()
			{
				curl_global_init (CURL_GLOBAL_ALL);
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::Release ()
			{
				curl_global_cleanup ();
			}

			QStringList Core::Provides () const
			{
				curl_version_info_data *data = curl_version_info (CURLVERSION_NOW);
				QStringList result ("ftp");
				if (data->age > 0)
				{
					if (data->features & CURL_VERSION_SSL)
						result << "http";
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
		};
	};
};

