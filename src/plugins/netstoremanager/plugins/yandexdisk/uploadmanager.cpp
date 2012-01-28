/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QBuffer>
#include <QFile>
#include <QtDebug>
#include "account.h"
#include "authmanager.h"
#include "urls.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	class OutDev : public QIODevice
	{
		QString Path_;

		qint64 Pos_;
		qint64 TotalSize_;

		QBuffer Preamble_;
		QFile File_;
		QBuffer End_;

		QByteArray Boundary_;
	public:
		OutDev (const QString& path, QObject *parent = 0)
		: QIODevice (parent)
		, Path_ (path)
		, Pos_ (0)
		, TotalSize_ (0)
		, File_ (path)
		, Boundary_ ("AaB03x")
		{
			Preamble_.buffer () += "--" + Boundary_ + "\r\n";
			Preamble_.buffer () += "Content-Disposition: form-data; name=\"file\"; filename=\"" +
					QFileInfo (path).fileName ().toUtf8 () + "\"\r\n";
			Preamble_.buffer () += "Content-Transfer-Encoding: binary\r\n";
			Preamble_.buffer () += "\r\n";

			End_.buffer () = "\r\n--" + Boundary_ + "--\r\n";

			TotalSize_ = Preamble_.size () + File_.size () + End_.size ();
		}

		QByteArray GetBoundary () const
		{
			return Boundary_;
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

			if (!File_.open (mode))
			{
				setErrorString ("File error: " + File_.errorString ());
				return false;
			}

			Preamble_.open (mode);
			End_.open (mode);

			return QIODevice::open (mode);
		}
	protected:
		qint64 readData (char *data, qint64 maxlen)
		{
			if (maxlen <= 0)
				return maxlen;

			auto& dev = GetDevice (Pos_);
			qint64 read = dev.read (data, maxlen);
			if (read == -1)
			{
				qWarning () << Q_FUNC_INFO
						<< "error reading device"
						<< &dev
						<< Pos_
						<< maxlen
						<< Preamble_.size ()
						<< File_.size ()
						<< End_.size ();
				return -1;
			}
			else if (!read)
				return 0;

			Pos_ += read;
			return read + readData (data + read, maxlen - read);
		}

		qint64 writeData (const char*, qint64)
		{
			return -1;
		}
	private:
		QIODevice& GetDevice (qint64 pos)
		{
			if (pos >= Preamble_.size () + File_.size ())
				return End_;
			else if (pos >= Preamble_.size ())
				return File_;
			else
				return Preamble_;
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

		emit statusChanged (tr ("Authenticating..."), Path_);
	}

	void UploadManager::handleGotCookies (const QList<QNetworkCookie>& cookies)
	{
		Mgr_->cookieJar ()->setCookiesFromUrl (cookies, UpURL);

		auto reply = Mgr_->get (A_->MakeRequest (QUrl ("http://narod.yandex.ru/disk/getstorage/")));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotStorage ()));

		emit statusChanged (tr ("Getting storage..."), Path_);
	}

	void UploadManager::handleGotStorage ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
		{
			emit gotError (tr ("Network error while getting storage: %1.")
						.arg (reply->errorString ()),
					Path_);
			emit finished ();
			return;
		}

		const QString& page = reply->readAll ();
		QRegExp rx ("\"url\":\"(\\S+)\".+\"hash\":\"(\\S+)\".+\"purl\":\"(\\S+)\"");
		if (rx.indexIn(page) < 0)
		{
			emit gotError (tr ("Error parsing server reply."), Path_);
			emit finished ();
			return;
		}

		emit statusChanged (tr ("Uploading file..."), Path_);
		QUrl upUrl (rx.cap (1) + "?tid=" + rx.cap (2));

		OutDev *dev = new OutDev (Path_, this);
		if (!dev->open (QIODevice::ReadOnly))
		{
			emit gotError (tr ("Error opening file."), Path_);
			emit finished ();
			return;
		}

		auto rq = A_->MakeRequest (upUrl);
		rq.setHeader (QNetworkRequest::ContentTypeHeader,
				"multipart/form-data, boundary=" + dev->GetBoundary ());
		rq.setHeader (QNetworkRequest::ContentLengthHeader,
				dev->size ());

		QNetworkReply *rep = Mgr_->post (rq, dev);
		dev->setParent (rep);
		connect (rep,
				SIGNAL (uploadProgress(qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
		connect (rep,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadFinished ()));
	}

	void UploadManager::handleUploadProgress (qint64 done, qint64 total)
	{
		emit uploadProgress (done, total, Path_);
	}

	void UploadManager::handleUploadFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
		{
			emit gotError (tr ("Error uploading file: %1.")
						.arg (reply->errorString ()),
					Path_);
			emit finished ();
			return;
		}

		emit statusChanged (tr ("Verifying..."), Path_);
		connect (Mgr_->get (A_->MakeRequest (QUrl ("http://narod.yandex.ru/disk/last/"))),
				SIGNAL (finished ()),
				this,
				SLOT (handleVerReqFinished ()));
	}

	void UploadManager::handleVerReqFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		if (reply->error () != QNetworkReply::NoError)
		{
			emit gotError (tr ("Error verifying upload: %1.")
						.arg (reply->errorString ()),
					Path_);
			emit finished ();
			return;
		}

		const QString& page = reply->readAll ();
		QRegExp rx ("<span class='b-fname'><a href=\"(http://narod.ru/disk/\\S+html)\">[^<]+</a></span><br/>");
		if (rx.indexIn(page) == -1)
		{
			emit gotError (tr ("Error verifying uploaded file."), Path_);
			emit finished ();
			return;
		}

		emit statusChanged (tr ("Uploaded successfully"), Path_);
		emit gotUploadURL (QUrl (rx.cap (1)), Path_);
		emit finished ();
	}
}
}
}
