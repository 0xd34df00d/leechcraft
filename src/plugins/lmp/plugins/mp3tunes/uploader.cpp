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

#include "uploader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QFile>
#include <QCryptographicHash>
#include <QtDebug>
#include <util/passutils.h>
#include <interfaces/lmp/icloudstorageplugin.h>
#include "consts.h"

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	Uploader::Uploader (const QString& login, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, NAM_ (nam)
	, FirstAuth_ (true)
	{
	}

	bool Uploader::Auth ()
	{
		const auto& pass = Util::GetPassword ("org.LeechCraft.LMP.MP3Tunes.Account." + Login_,
				tr ("Enter password for MP3tunes account %1:").arg (Login_),
				this,
				FirstAuth_);
		if (pass.isEmpty ())
			return false;

		const auto authUrl = QString ("https://shop.mp3tunes.com/api/v1/login?output=xml&"
				"username=%1&password=%2&partner_token=%3")
					.arg (Login_)
					.arg (pass)
					.arg (Consts::PartnerId);
		auto reply = NAM_->get (QNetworkRequest (authUrl));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAuthReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleAuthReplyError ()));

		return true;
	}

	void Uploader::Upload (const QString& up)
	{
		if (SID_.isEmpty ())
		{
			UpAfterAuth_ = up;
			Auth ();
			return;
		}

		QFile file (up);
		if (!file.open (QIODevice::ReadOnly))
		{
			emit uploadFinished (UpAfterAuth_,
					CloudStorageError::LocalError,
					tr ("Unable to open file %1 for reading.")
						.arg (up));
			return;
		}

		const auto& fileData = file.readAll ();
		file.close ();

		const auto& hash = QCryptographicHash::hash (fileData, QCryptographicHash::Md5);
		const auto url = QString ("http://content.mp3tunes.com/storage/lockerPut/%1?"
				"output=xml&sid=%2&partner_token=%3")
					.arg (QString (hash.toHex ()))
					.arg (SID_)
					.arg (Consts::PartnerId);

		auto reply = NAM_->put (QNetworkRequest (url), fileData);
		reply->setProperty ("Filename", up);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadFinished ()));
		qDebug () << Q_FUNC_INFO << "sent request for" << up;
	}

	void Uploader::handleAuthReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			emit uploadFinished (UpAfterAuth_,
					CloudStorageError::ServiceError,
					tr ("Unable to parse authentication reply."));
			return;
		}

		const auto& docElem = doc.documentElement ();
		if (docElem.firstChildElement ("status").text () != "1")
		{
			FirstAuth_ = false;
			emit uploadFinished (UpAfterAuth_,
					CloudStorageError::NotAuthorized,
					docElem.firstChildElement ("errorMessage").text ());
			return;
		}

		SID_ = docElem.firstChildElement ("session_id").text ();
		FirstAuth_ = true;

		if (!UpAfterAuth_.isEmpty ())
		{
			Upload (UpAfterAuth_);
			UpAfterAuth_.clear ();
		}
	}

	void Uploader::handleAuthReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		emit uploadFinished (UpAfterAuth_,
				CloudStorageError::NetError,
				tr ("Unable to parse authentication reply."));
	}

	void Uploader::handleUploadFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QMap<QByteArray, CloudStorageError> errors;
		errors ["400002"] = CloudStorageError::FileHashMismatch;
		errors ["401002"] = CloudStorageError::InvalidSession;
		errors ["401003"] = CloudStorageError::NotAuthorized;
		errors ["401005"] = CloudStorageError::InvalidSession;
		errors ["403001"] = CloudStorageError::OtherError;
		errors ["403002"] = CloudStorageError::UnsupportedFileFormat;
		errors ["409001"] = CloudStorageError::FileHashMismatch;
		errors ["413001"] = CloudStorageError::FilesizeExceeded;
		errors ["413002"] = CloudStorageError::StorageFull;
		errors ["415001"] = CloudStorageError::UnsupportedFileFormat;
		errors ["500001"] = CloudStorageError::ServiceError;
		errors ["503001"] = CloudStorageError::ServiceError;

		const auto& errnoHeader = reply->rawHeader ("X-MP3tunes-ErrorNo");
		const auto& errMsgHeader = reply->rawHeader ("X-MP3tunes-ErrorString");

		qDebug () << Q_FUNC_INFO << errnoHeader << errMsgHeader;
		emit uploadFinished (reply->property ("Filename").toString (),
				errors.value (errnoHeader, CloudStorageError::NoError),
				errMsgHeader);
	}
}
}
}
