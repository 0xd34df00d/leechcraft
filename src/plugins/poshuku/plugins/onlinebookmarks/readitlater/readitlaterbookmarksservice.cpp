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

#include "readitlaterbookmarksservice.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "../core.h"

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
	const QString ApiKey = "0l7A6m89daNpif742cpM7fRJe9Tcxd49";
	
	const QString ServiceUrl = "http://readitlaterlist.com";

	const QString LoginUrl = "https://readitlaterlist.com/v2/auth?";
	const QString GetBookmarksUrl = "https://readitlaterlist.com/v2/get?";
	const QString SetBookmarksUrl = "https://readitlaterlist.com/v2/send";
	
	const QString AuthOk = "200";
	
	const QString name = "title";
	const QString link = "url";
	const QString tag = "tags";
	
	ReadItLaterBookmarksService::ReadItLaterBookmarksService (QWidget *parent)
	: ApiUrl_ (QUrl (LoginUrl))
	{
	}

	QString ReadItLaterBookmarksService::GetName () const
	{
		return QString ("Read It Later");
	}

	QIcon ReadItLaterBookmarksService::GetIcon () const
	{
		return QIcon (":plugins/poshuku/plugins/readitlater/resources/images/readitlater.ico");
	}

	void ReadItLaterBookmarksService::CheckValidAccountData (const QString& login, const QString& pass)
	{
		Type_ = Auth_;
		
		QString loginString = "username=" + login + 
				"&password=" + pass + 
				"&apikey=" + ApiKey;
		
		RequestString_ = QByteArray (loginString.toUtf8 ());
		QNetworkRequest request (ApiUrl_);
		Reply_ =  Core::Instance ().GetNetworkAccessManager ()->post (request, RequestString_);

		connect (Reply_,
				SIGNAL (finished ()),
				this,
				SLOT (getReplyFinished ()));

		connect (Reply_,
				SIGNAL (readyRead ()),
				this,
				SLOT (readyReadReply ()));
	}
	
	void ReadItLaterBookmarksService::DownloadBookmarks (const QStringList& logins, int lastDownloadTime)
	{
		Type_ = Download_;
		Q_FOREACH (const QString& login, logins)
		{
			QString password = Core::Instance ().GetPassword (login, "Read It Later");
			if (password.isNull ())
			{
				gotParseError (tr ("Invalid password"));
				return;
			}
			FetchBookmarks (login, password, lastDownloadTime);
		}
	}

	void ReadItLaterBookmarksService::UploadBookmarks (const QStringList& logins, const QList<QVariant>& bookamrks)
	{
		Type_ = Upload_;
		Q_FOREACH (const QString& login, logins)
		{
			QString password = Core::Instance ().GetPassword (login, "Read It Later");
			if (password.isNull ())
			{
				gotParseError (tr ("Invalid password"));
				return;
			}
			
			SendBookmarks (login, password, bookamrks);
		}
	}


	void ReadItLaterBookmarksService::getReplyFinished ()
	{
		Reply_->deleteLater ();
	}

	void ReadItLaterBookmarksService::readyReadReply ()
	{
		switch (Type_)
		{
		case Auth_:
			emit gotValidReply ((Reply_->attribute (QNetworkRequest::HttpStatusCodeAttribute) == 200) 
					? true : false);
			break;
		case Download_:
			ParseDownloadReply (Reply_->readAll ());
			break;
		case Upload_:
			ParseUploadReply ((Reply_->attribute (QNetworkRequest::HttpStatusCodeAttribute) == 200));
			break;
		case Sync_:
			break;
		}
	}
	
		void ReadItLaterBookmarksService::FetchBookmarks (const QString& login, const QString& pass, int lastDownloadTime)
	{
		ApiUrl_ = GetBookmarksUrl + 
				"username=" + login + 
				"&password=" + pass + 
				"&apikey=" + ApiKey + 
				"&since=" + lastDownloadTime +
				"&tags=1";
		QNetworkRequest request (ApiUrl_);
		Reply_ = Core::Instance ().GetNetworkAccessManager ()->get (request);
		connect (Reply_, 
				SIGNAL (finished ()),
				this, 
				SLOT (getReplyFinished ()));
		
		connect (Reply_, 
				SIGNAL (readyRead ()), 
				this, 
				SLOT (readyReadReply ()));
	}

	void ReadItLaterBookmarksService::SendBookmarks(const QString& login, const QString& pass, 
			const QList< QVariant >& bookmarks)
	{
		QVariantMap exportBookmarks, exportTags;
		int i = 0;
		int j = 0;
		Q_FOREACH (const QVariant& record, bookmarks)
		{
			QVariantMap bookmark, tags;
			bookmark.insert ("url", record.toMap () ["URL"].toString ());
			bookmark.insert ("title", record.toMap () ["Title"].toString ());
			
			if (!(record.toMap () ["Tags"].toStringList ().isEmpty ()))
			{
				tags.insert ("url", record.toMap () ["URL"].toString ());
				tags.insert ("tags", record.toMap () ["Tags"].toString ());
				exportTags.insert (QString::number (j), tags);
				j++;
			}
			exportBookmarks.insert(QString::number (i), bookmark);
			i++;
		}
		
		QJson::Serializer serializer;
		QByteArray jsonBookmarks = serializer.serialize (exportBookmarks);
		QByteArray jsonTags = serializer.serialize (exportTags);
		qDebug () << jsonTags;
		ApiUrl_ = SetBookmarksUrl +
				"?username=" + login + 
				"&password=" + pass +
				"&apikey=" + ApiKey +
				"&new=" + QString::fromUtf8 (jsonBookmarks.constData ()) + 
				"&update_tags=" + QString::fromUtf8 (jsonTags.constData ());
		
// 		QNetworkRequest request (ApiUrl_);
// 		Reply_ = Core::Instance ().GetNetworkAccessManager ()->get (request);
// 		
// 		connect (Reply_, 
// 				SIGNAL (finished ()),
// 				this, 
// 				SLOT (getReplyFinished ()));
// 		
// 		connect (Reply_, 
// 				SIGNAL (readyRead ()), 
// 				this, 
// 				SLOT (readyReadReply ()));
	}

	void ReadItLaterBookmarksService::ParseDownloadReply (const QByteArray& reply)
	{
		QJson::Parser parser;
		bool ok;
		
		const QVariantMap& result = parser.parse (reply, &ok).toMap ();
		
		if (!ok)
		{
			emit gotParseError (tr ("An error occurred during parsing"));
			return;
		}

		const QVariantMap& nestedMap = result ["list"].toMap ();
		
		QList<QVariant> bookmarks;
		Q_FOREACH (const QVariant& var, nestedMap)
		{
			QMap<QString, QVariant> record;
			QMap<QString, QVariant> map = var.toMap ();
			
			record ["Tags"] = map [tag].toStringList ();
			record ["Title"] = map [name].toString ();
			record ["URL"] = map [link].toString ();
			
			bookmarks.push_back (record);
		}
		
		emit gotDownloadReply (bookmarks, QUrl (ServiceUrl));
	}
	
	void ReadItLaterBookmarksService::ParseUploadReply (bool code)
	{
		emit gotUploadReply (code);
	}

}
}
}
}
}

