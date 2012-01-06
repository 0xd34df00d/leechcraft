/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "uploadmanager.h"
#include <stdexcept>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileInfo>
#include <QRegExp>
#include <QFile>
#include "account.h"
#include "authmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	const QUrl UpURL = QUrl ("http://narod.yandex.ru");
	const QUrl AuthURL = QUrl ("http://passport.yandex.ru/passport?mode=auth");

	class OutDev : public QIODevice
	{
		QString Path_;

		qint64 Pos_;
		qint64 TotalSize_;

		QByteArray Preamble_;
		QFile *File_;
		QByteArray End_;
	public:
		OutDev (const QString& path, QObject *parent = 0)
		: QIODevice (parent)
		, Path_ (path)
		, Pos_ (0)
		, TotalSize_ (0)
		, File_ (new QFile (path, this))
		{
			const QByteArray boundary = "AaB03x";

			Preamble_ += "--" + boundary + "\r\n";
			Preamble_ += "Content-Disposition: form-data; name=\"file\"; filename=\"" +
					QFileInfo (path).fileName ().toUtf8 () + "\"\r\n";
			Preamble_ += "Content-Transfer-Encoding: binary\r\n";
			Preamble_ += "\r\n";

			End_ = "\r\n--" + boundary + "--\r\n";

			TotalSize_ = Preamble_.size () + File_->size () + End_.size ();
		}

		qint64 size () const
		{
			return TotalSize_;
		}

		bool seek (qint64 pos)
		{
			if (pos >= TotalSize_)
				return false;

			Pos_ = pos;
			return QIODevice::seek (pos);
		}

		bool open (OpenMode mode)
		{
			if (mode != QIODevice::ReadOnly)
			{
				setErrorString ("Unable to open the device in non-read-only mode");
				return false;
			}

			if (!File_->open (mode))
			{
				setErrorString ("File error: " + File_->errorString ());
				return false;
			}

			return QIODevice::open (mode);
		}
	protected:
		qint64 readData (char *data, qint64 maxlen)
		{
			// TODO
		}

		qint64 writeData (const char*, qint64)
		{
			return -1;
		}
	};

	UploadManager::UploadManager (const QString& filepath, Account *acc)
	: QObject (acc)
	, A_ (acc)
	, Mgr_ (new QNetworkAccessManager (this))
	, Path_ (filepath)
	{
		connect (this,
				SIGNAL (finished ()),
				this,
				SLOT (deleteLater ()),
				Qt::QueuedConnection);

		auto am = acc->GetAuthManager ();
		connect (am,
				SIGNAL (gotCookies (QList<QNetworkCookie>)),
				this,
				SLOT (handleGotCookies (QList<QNetworkCookie>)));

		am->GetCookiesFor (acc->GetLogin (), acc->GetPassword ());

		emit statusChanged (tr ("Authenticating..."));
	}

	void UploadManager::handleGotCookies (const QList<QNetworkCookie>& cookies)
	{
		Mgr_->cookieJar ()->setCookiesFromUrl (cookies, UpURL);

		QNetworkRequest rq (QUrl ("http://narod.yandex.ru/disk/getstorage/"));
		rq.setRawHeader ("Cache-Control", "no-cache");
		rq.setRawHeader ("Accept", "*/*");
		auto reply = Mgr_->get (rq);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotStorage ()));

		emit statusChanged (tr ("Getting storage..."));
	}

	void UploadManager::handleGotStorage ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
		{
			emit gotError (tr ("Network error while getting storage: %1.")
						.arg (reply->errorString ()));
			emit finished ();
			return;
		}

		const QString& page = reply->readAll ();
		QRegExp rx ("\"url\":\"(\\S+)\".+\"hash\":\"(\\S+)\".+\"purl\":\"(\\S+)\"");
		if (rx.indexIn(page) < 0)
		{
			emit gotError (tr ("Error parsing server reply."));
			emit finished ();
		}

		emit statusChanged (tr ("Uploading file..."));
		QUrl upUrl (rx.cap (1) + "?tid=" + rx.cap (2));

		OutDev *dev = new OutDev (Path_);
	}
}
}
}
