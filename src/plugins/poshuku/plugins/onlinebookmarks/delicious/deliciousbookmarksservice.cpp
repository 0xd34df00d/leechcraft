/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "deliciousbookmarksservice.h"
#include <QStringList>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#define LOGIN_URL "https://secure.delicious.com/login"
#define AUTH_OK "secure.del.ac4.yahoo.net uncompressed/chunked"


namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace OnlineBookmarks
				{
					DeliciousBookmarksService::DeliciousBookmarksService (QWidget *parent)
					:ApiUrl_ (QUrl (LOGIN_URL))
					{
					}

					QString DeliciousBookmarksService::GetName () const
					{
						return QString ("Del.icio.us");
					}
					
					QIcon DeliciousBookmarksService::GetIcon () const
					{
						return QIcon (":delicious/resources/images/delicious.png");
					}
					
					void DeliciousBookmarksService::CheckValidAccountData (const QString& login, const QString& pass)
					{
						QString loginString = "username=" + login + "&password=" + pass;
						RequestString_ = QByteArray (loginString.toStdString ().c_str ());
						QNetworkRequest request (ApiUrl_);
						Reply_ = Manager_.post (request, RequestString_);
						
						connect (Reply_, 
								SIGNAL (finished ()),
								this,
								SLOT (getReplyFinished ()));
						
						connect (Reply_, 
								SIGNAL (readyRead ()),
								this, 
								SLOT (readyReadReply ()));
					}
					
					void DeliciousBookmarksService::SetYahooID (bool yahooId)
					{
						YahooID_ = yahooId;
					}
					
					void DeliciousBookmarksService::getReplyFinished ()
					{
						Reply_->deleteLater ();
					}

					void DeliciousBookmarksService::readyReadReply ()
					{
						emit getValidReply (QString::fromUtf8 (Reply_->readAll ()).
								contains (AUTH_OK, Qt::CaseInsensitive));
					}
				};
			};
		};
	};
};