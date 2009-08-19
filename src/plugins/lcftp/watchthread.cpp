/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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
							&tv);

					emit shouldPerform ();

					msleep (100);
				}
			}
		};
	};
};

