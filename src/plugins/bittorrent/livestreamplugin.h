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

#ifndef PLUGINS_BITTORRENT_LIVESTREAMPLUGIN_H
#define PLUGINS_BITTORRENT_LIVESTREAMPLUGIN_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <libtorrent/torrent.hpp>
#include <libtorrent/extensions.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class LiveStreamPlugin : public QObject
								   , public libtorrent::torrent_plugin
			{
				Q_OBJECT

				void *Data_;
				libtorrent::torrent *Torrent_;
			public:
				LiveStreamPlugin (libtorrent::torrent*, void*);

				virtual void on_piece_pass (int);
			signals:
				void gotPiece (int, void*);
			};

			boost::shared_ptr<LiveStreamPlugin> LiveStreamPluginFactory (libtorrent::torrent*, void*);
		};
	};
};

#endif

