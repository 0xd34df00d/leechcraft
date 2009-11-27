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

#ifndef PLUGINS_POSHUKU_PLUGINS_WYFV_PLAYER_H
#define PLUGINS_POSHUKU_PLUGINS_WYFV_PLAYER_H
#include <boost/function.hpp>
#include <QWidget>
#include <QList>
#include <interfaces/imediaplayer.h>
#include "ui_player.h"

class QNetworkRequest;
class QNetworkAccessManager;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace WYFV
				{
					class Player : public QWidget
					{
						Q_OBJECT

						IVideoWidget *Player_;
					protected:
						QNetworkAccessManager *ClearNAM_;
						Ui::Player Ui_;

						Player (const QUrl&, const QStringList&, const QStringList&);
						void SetVideoUrl (const QUrl&);
						void SetRequest (const QNetworkRequest&);
					public:
						virtual ~Player ();
					};
				};
			};
		};
	};
};

#endif

