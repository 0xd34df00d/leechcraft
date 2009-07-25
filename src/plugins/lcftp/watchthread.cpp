#include "watchthread.h"
#include <curl/curl.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			WatchThread::WatchThread (QObject *parent)
			: QThread (parent)
			, Quitting_ (false)
			{
			}

			void WatchThread::SetExit ()
			{
				Quitting_ = true;
			}

			void WatchThread::run ()
			{
				fd_set read_fds, write_fds, error_fds;
				int nfds;
				long timeout;
				struct timeval tv;

				while (!Quitting_)
				{
					FD_ZERO (&read_fds);
					FD_ZERO (&write_fds);
					FD_ZERO (&error_fds);

					while (true)
					{
						CURLMcode result;
						{
							QMutexLocker l (&Core::Instance ().MultiHandleMutex_);
							result = curl_multi_fdset (Core::Instance ().MultiHandle_.get (),
									&read_fds,
									&write_fds,
									&error_fds,
									&nfds);
						}

						if ((!result &&
								nfds != -1) || 
								Quitting_)
							break;

						msleep (500);
					}

					if (Quitting_)
						return;

					tv.tv_sec = 1;
					tv.tv_usec = 0;

					select (nfds + 1,
							&read_fds,
							&write_fds,
							&error_fds,
							timeout < 0 ? NULL : &tv);

					emit shouldPerform ();

					msleep (100);
				}
			}
		};
	};
};

