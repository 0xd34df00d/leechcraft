/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattabnetworkaccessmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <QBuffer>
#include <util/sll/delayedexecutor.h>
#include <util/threads/futures.h>
#include "avatarsmanager.h"
#include "core.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		class AvatarReply final : public QNetworkReply
		{
			QBuffer Buffer_;
		public:
			AvatarReply (const QNetworkRequest&, AvatarsManager*);

			qint64 bytesAvailable () const override;
			qint64 readData (char* data, qint64 maxlen) override;
			void abort () override;
		private:
			void HandleImage (const QImage&);
		};

		AvatarReply::AvatarReply (const QNetworkRequest& req, AvatarsManager *am)
		{
			open (QIODevice::ReadOnly);

			setHeader (QNetworkRequest::ContentTypeHeader, QByteArray { "image/png" });
			setAttribute (QNetworkRequest::HttpStatusCodeAttribute, 200);
			setAttribute (QNetworkRequest::HttpReasonPhraseAttribute, QByteArray { "OK" });

			Util::ExecuteLater ([this] { emit metaDataChanged (); });

			const auto& entryIdPath = req.url ().path ().section ('/', 1, 1);
			const auto& entryId = QString::fromUtf8 (QByteArray::fromBase64 (entryIdPath.toLatin1 ()));

			const auto entryObj = Core::Instance ().GetEntry (entryId);
			if (!entryObj)
			{
				Util::ExecuteLater ([this]
						{ HandleImage (ResourcesManager::Instance ().GetDefaultAvatar (32)); });
				return;
			}

			Util::Sequence (this, am->GetAvatar (entryObj, IHaveAvatars::Size::Thumbnail)) >>
					[this] (const QImage& image) { HandleImage (image); };
		}

		qint64 AvatarReply::bytesAvailable () const
		{
			return QNetworkReply::bytesAvailable () + Buffer_.bytesAvailable ();
		}

		qint64 AvatarReply::readData (char *data, qint64 maxlen)
		{
			return Buffer_.read (data, maxlen);
		}

		void AvatarReply::abort ()
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot abort";
		}

		void AvatarReply::HandleImage (const QImage& image)
		{
			Buffer_.open (QIODevice::WriteOnly);
			image.save (&Buffer_, "PNG", 100);
			Buffer_.close ();

			Buffer_.open (QIODevice::ReadOnly);

			setHeader (QNetworkRequest::ContentLengthHeader, Buffer_.bytesAvailable ());
			emit downloadProgress (Buffer_.size (), Buffer_.size ());

			emit readyRead ();

			emit finished ();
		}
	}

	ChatTabNetworkAccessManager::ChatTabNetworkAccessManager (AvatarsManager *am, QObject *parent)
	: QNetworkAccessManager { parent }
	, AvatarsMgr_ { am }
	{
	}

	QNetworkReply* ChatTabNetworkAccessManager::createRequest (Operation op,
			const QNetworkRequest& request, QIODevice *outgoingData)
	{
		const auto& url = request.url ();
		if (url.scheme () == "azoth" && url.host () == "avatar")
			return new AvatarReply { request, AvatarsMgr_ };

		return QNetworkAccessManager::createRequest (op, request, outgoingData);
	}
}
}
