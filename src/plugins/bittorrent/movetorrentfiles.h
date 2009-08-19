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

#ifndef PLUGINS_BITTORRENT_MOVETORRENTFILES_H
#define PLUGINS_BITTORRENT_MOVETORRENTFILES_H
#include <QDialog>
#include "ui_movetorrentfiles.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class MoveTorrentFiles : public QDialog
			{
				Q_OBJECT

				Ui::MoveTorrentFiles Ui_;
			public:
				MoveTorrentFiles (const QString&, QWidget* = 0);
				QString GetNewLocation () const;
			private slots:
				void on_Browse__released ();
			};
		};
	};
};

#endif

