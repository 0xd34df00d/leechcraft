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
		enum class CloudStorageError
		{
			NetError,
			FileHashMismatch,
			InvalidSession,
			NotAuthorized,
			UnsupportedFileFormat,
			FilesizeExceeded,
			StorageFull,
			ServiceError,
			OtherError
		};

		QString GetErrorString (CloudStorageError error)
		{
			switch (error)
			{
			case CloudStorageError::NetError:
				return Uploader::tr ("network error");
			case CloudStorageError::FileHashMismatch:
				return Uploader::tr ("file hash mismatch");
			case CloudStorageError::InvalidSession:
				return Uploader::tr ("invalid session");
			case CloudStorageError::NotAuthorized:
				return Uploader::tr ("not authorized");
			case CloudStorageError::UnsupportedFileFormat:
				return Uploader::tr ("unsupported file format");
			case CloudStorageError::FilesizeExceeded:
				return Uploader::tr ("file size exceeded");
			case CloudStorageError::StorageFull:
				return Uploader::tr ("storage full");
			case CloudStorageError::ServiceError:
				return Uploader::tr ("service error");
			case CloudStorageError::OtherError:
				return Uploader::tr ("unknown error");
			}

			std::unreachable ();
		}

		CloudStorageError GetErrorCode (const QByteArray& code)
		{
			static const QHash<QByteArray, CloudStorageError> errors
			{
				{ "400002"_qba, CloudStorageError::FileHashMismatch },
				{ "401002"_qba, CloudStorageError::InvalidSession },
				{ "401003"_qba, CloudStorageError::NotAuthorized },
				{ "401005"_qba, CloudStorageError::InvalidSession },
				{ "403001"_qba, CloudStorageError::OtherError },
				{ "403002"_qba, CloudStorageError::UnsupportedFileFormat },
				{ "409001"_qba, CloudStorageError::FileHashMismatch },
				{ "413001"_qba, CloudStorageError::FilesizeExceeded },
				{ "413002"_qba, CloudStorageError::StorageFull },
				{ "415001"_qba, CloudStorageError::UnsupportedFileFormat },
				{ "500001"_qba, CloudStorageError::ServiceError },
				{ "503001"_qba, CloudStorageError::ServiceError },
			};
			return errors.value (code, CloudStorageError::ServiceError);
		}
	}

	Util::ContextTask<ISyncPlugin::UploadResult> Uploader::Upload (const QString& path)
	{
		co_await Util::AddContextObject { *this };
		const auto sid = co_await co_await AuthMgr_->GetSID (Login_);

		QFile file { path };
		if (!file.open (QIODevice::ReadOnly))
		{
			const auto& userMsg = tr ("Unable to open file %1 for reading.").arg (path);
			co_return { Util::AsLeft, { QFile::OpenError, userMsg } };
		}

		const auto& fileData = file.readAll ();
		const auto& hash = QCryptographicHash::hash (fileData, QCryptographicHash::Md5);
		const auto url = "https://content.mp3tunes.com/storage/lockerPut/%1?output=xml&sid=%2&partner_token=%3"_qs
				.arg (hash.toHex (), sid, Consts::PartnerId);

		qDebug () << "sending request for" << path;
		const auto reply = GetProxyHolder ()->GetNetworkAccessManager ()->put (QNetworkRequest { url }, fileData);
		const auto response = co_await *reply;
		if (!response.IsError ())
			co_return ISyncPlugin::UploadSuccess {};

		const auto& errnoHeader = reply->rawHeader ("X-MP3tunes-ErrorNo");
		const auto& errMsgHeader = reply->rawHeader ("X-MP3tunes-ErrorString");
		const auto errorCode = GetErrorCode (errnoHeader);
		const auto errorString = GetErrorString (errorCode);
		co_return { Util::AsLeft, { QFile::CopyError, tr ("uploading failed: %1 (%2)").arg (errorString, errMsgHeader) } };
	}
}
