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
					qDebug () << Q_FUNC_INFO
						<< "on"
						<< QString::fromUtf8 (handle.save_path ().string ().c_str ());
					LiveStreamDevice *lsd = new LiveStreamDevice (handle, this);
					Handle2Device_ [handle] = lsd;
					connect (lsd,
							SIGNAL (ready ()),
							this,
							SLOT (handleDeviceReady ()));
					lsd->CheckReady ();
				}
			}

			bool LiveStreamManager::IsEnabledOn (libtorrent::torrent_handle handle)
			{
				return Handle2Device_.contains (handle);
			}

			void LiveStreamManager::PieceRead (const libtorrent::read_piece_alert& a)
			{
				libtorrent::torrent_handle handle =
					a.handle;

				if (!Handle2Device_.contains (handle))
				{
					qWarning () << Q_FUNC_INFO
						<< "Handle2Device_ doesn't contain handle"
						<< Handle2Device_.size ();
					return;
				}

				Handle2Device_ [handle]->PieceRead (a);
			}

			void LiveStreamManager::handleDeviceReady ()
			{
				LiveStreamDevice *lsd = qobject_cast<LiveStreamDevice*> (sender ());
				if (!lsd)
				{
					qWarning () << Q_FUNC_INFO
						<< "sender() is not a LiveStreamDevice"
						<< sender ();
					return;
				}

				DownloadEntity e;
				e.Entity_ = QVariant::fromValue<QIODevice*> (lsd);
				e.Parameters_ = FromUserInitiated;
				e.Mime_ = "x-leechcraft/media-qiodevice";
				emit gotEntity (e);
			}
		};
	};
};

