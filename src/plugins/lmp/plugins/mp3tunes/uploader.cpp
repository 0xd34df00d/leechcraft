/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QFile>
#include <QCryptographicHash>
#include <QtDebug>
#include <interfaces/lmp/icloudstorageplugin.h>
#include "authmanager.h"
#include "consts.h"

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	Uploader::Uploader (const QString& login, QNetworkAccessManager *nam, AuthManager *auth, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, NAM_ (nam)
	, AuthMgr_ (auth)
	{
		connect (AuthMgr_,
				SIGNAL (sidReady (QString)),
				this,
				SLOT (handleSidReady (QString)));
		connect (AuthMgr_,
				SIGNAL (sidError (QString, QString)),
				this,
				SLOT (handleSidError (QString, QString)));
	}

	void Uploader::Upload (const QString& up)
	{
		const auto& sid = AuthMgr_->GetSID (Login_);
		if (sid.isEmpty ())
		{
			UpAfterAuth_ = up;
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
					.arg (sid)
					.arg (Consts::PartnerId);

		auto reply = NAM_->put (QNetworkRequest (url), fileData);
		reply->setProperty ("Filename", up);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadFinished ()));
		qDebug () << Q_FUNC_INFO << "sent request for" << up;
	}

	void Uploader::handleSidReady (const QString& login)
	{
		if (login != Login_)
			return;

		Upload (UpAfterAuth_);
		UpAfterAuth_.clear ();
	}

	void Uploader::handleSidError (const QString& login, const QString& msg)
	{
		if (login != Login_)
			return;

		emit uploadFinished (UpAfterAuth_,
					CloudStorageError::NotAuthorized,
					msg);

		UpAfterAuth_.clear ();
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
