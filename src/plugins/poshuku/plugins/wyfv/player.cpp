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

#include "player.h"
#include <QNetworkReply>
#include <QMessageBox>
#include "core.h"

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
					Player::Player (const QUrl&,
							const QStringList&,
							const QStringList&)
					: Player_ (0)
					, ClearNAM_ (new QNetworkAccessManager)
					{
						Ui_.setupUi (this);
						QList<IMediaPlayer*> players = Core::Instance ().GetProxy ()->
							GetPluginsManager ()->GetAllCastableTo<IMediaPlayer*> ();
						Q_FOREACH (IMediaPlayer *player, players)
							if ((Player_ = player->CreateWidget ()))
								break;
						if (Player_)
						{
							qobject_cast<QBoxLayout*> (layout ())->
								insertWidget (0, Player_->Widget ());
							connect (Player_->Widget (),
									SIGNAL (error (const QString&)),
									this,
									SLOT (handlePlayerError (const QString&)));
						}
					}

					Player::~Player ()
					{
						delete Player_;
						delete ClearNAM_;
					}

					void Player::SetVideoUrl (const QUrl& url)
					{
						if (Player_)
						{
							Player_->Stop ();
							Player_->Clear ();
							Player_->Enqueue (url);
							Player_->Play ();
						}
					}

					void Player::SetRequest (const QNetworkRequest& req)
					{
						if (Player_)
						{
							QNetworkReply *rep = ClearNAM_->get (req);
							Player_->Stop ();
							Player_->Clear ();
							Player_->Enqueue (rep);
							Player_->Play ();
						}
					}

					void Player::handlePlayerError (const QString& error)
					{
						QMessageBox::critical (this,
								tr ("LeechCraft"),
								error);
					}
				};
			};
		};
	};
};

