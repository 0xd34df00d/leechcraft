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

#include "livestreamplugin.h"
#include "livestreammanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			LiveStreamPlugin::LiveStreamPlugin (libtorrent::torrent *torrent,
					void *data)
			: Data_ (data)
			, Torrent_ (torrent)
			{
				LiveStreamManager *lsm = static_cast<LiveStreamManager*> (Data_);
				connect (this,
						SIGNAL (gotPiece (int, void*)),
						lsm,
						SLOT (handleGotPiece (int, void*)));
			}

			void LiveStreamPlugin::on_piece_pass (int index)
			{
				emit gotPiece (index, Torrent_);
			}

			boost::shared_ptr<LiveStreamPlugin> LiveStreamPluginFactory (libtorrent::torrent *t,
					void *v)
			{
				return boost::shared_ptr<LiveStreamPlugin> (new LiveStreamPlugin (t, v));
			}
		};
	};
};

