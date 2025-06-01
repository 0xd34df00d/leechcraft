/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "uploader.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QCryptographicHash>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include "authmanager.h"
#include "consts.h"

namespace LC::LMP::MP3Tunes
{
	Uploader::Uploader (const QString& login, AuthManager *auth, QObject *parent)
	: QObject { parent }
	, Login_ { login }
	, AuthMgr_ { auth }
	{
	}

	namespace
	{
		CloudStorageError GetErrorCode (const QByteArray& code)
		{
			static const QHash<QByteArray, CloudStorageError> errors
			{
				{ "400002", CloudStorageError::FileHashMismatch },
				{ "401002", CloudStorageError::InvalidSession },
				{ "401003", CloudStorageError::NotAuthorized },
				{ "401005", CloudStorageError::InvalidSession },
				{ "403001", CloudStorageError::OtherError },
				{ "403002", CloudStorageError::UnsupportedFileFormat },
				{ "409001", CloudStorageError::FileHashMismatch },
				{ "413001", CloudStorageError::FilesizeExceeded },
				{ "413002", CloudStorageError::StorageFull },
				{ "415001", CloudStorageError::UnsupportedFileFormat },
				{ "500001", CloudStorageError::ServiceError },
				{ "503001", CloudStorageError::ServiceError },
			};
			return errors.value (code, CloudStorageError::ServiceError);
		}
	}

	Util::ContextTask<ICloudStoragePlugin::UploadResult> Uploader::Upload (const QString& path)
	{
		co_await Util::AddContextObject { *this };
		const auto eitherSid = co_await AuthMgr_->GetSID (Login_);
		const auto sid = co_await eitherSid;

		QFile file { path };
		if (!file.open (QIODevice::ReadOnly))
		{
			const auto& userMsg = tr ("Unable to open file %1 for reading.").arg (path);
			co_return { Util::AsLeft, { CloudStorageError::LocalError, userMsg } };
		}

		const auto& fileData = file.readAll ();
		const auto& hash = QCryptographicHash::hash (fileData, QCryptographicHash::Md5);
		const auto url = "https://content.mp3tunes.com/storage/lockerPut/%1?output=xml&sid=%2&partner_token=%3"_qs
				.arg (hash.toHex (), sid, Consts::PartnerId);

		qDebug () << "sending request for" << path;
		const auto reply = GetProxyHolder ()->GetNetworkAccessManager ()->put (QNetworkRequest { url }, fileData);
		const auto response = co_await *reply;
		if (!response.IsError ())
			co_return Util::Void {};

		const auto& errnoHeader = reply->rawHeader ("X-MP3tunes-ErrorNo");
		const auto& errMsgHeader = reply->rawHeader ("X-MP3tunes-ErrorString");
		co_return { Util::AsLeft, { GetErrorCode (errnoHeader), tr ("uploading failed: %1").arg (errMsgHeader) } };
	}
}
