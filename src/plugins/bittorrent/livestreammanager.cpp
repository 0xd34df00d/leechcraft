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

#include "livestreammanager.h"
#include "livestreamplugin.h"
#include "livestreamdevice.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			LiveStreamManager::LiveStreamManager (QObject *parent)
			: QObject (parent)
			{
			}

			void LiveStreamManager::EnableOn (libtorrent::torrent_handle handle)
			{
				if (!Handle2Device_.contains (handle))
				{
					handle.add_extension (&LiveStreamPluginFactory, this);

					const libtorrent::torrent_info& ti = handle.get_torrent_info ();
					int size = ti.num_pieces ();
					Handle2Device_ [handle] =
						new LiveStreamDevice (handle);
				}
			}

			void LiveStreamManager::PieceRead (const libtorrent::read_piece_alert& a)
			{
				libtorrent::torrent_handle handle =
					a.handle;
				Handle2Device_ [handle]->PieceRead (a);
			}

			void LiveStreamManager::handleGotPiece (int index, void *tp)
			{
				libtorrent::torrent_handle handle =
					static_cast<libtorrent::torrent*> (tp)->get_handle ();
				Handle2Device_ [handle]->GotPiece (index);
			}
		};
	};
};

