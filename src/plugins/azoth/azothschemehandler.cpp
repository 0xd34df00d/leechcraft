/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "azothschemehandler.h"
#include <QBuffer>
#include <QImage>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>
#include <QWebEngineUrlSchemeHandler>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include "avatarsmanager.h"
#include "core.h"
#include "resourcesmanager.h"

namespace
{
	[[maybe_unused]] const int SchemeRegistrar = []
	{
		QWebEngineUrlScheme azothScheme { "azoth" };
		azothScheme.setSyntax (QWebEngineUrlScheme::Syntax::Host);
		QWebEngineUrlScheme::registerScheme (azothScheme);
		return 0;
	} ();
}

namespace LC::Azoth
{
	AzothSchemeHandler::AzothSchemeHandler (AvatarsManager *am, QObject *parent)
	: QWebEngineUrlSchemeHandler { parent }
	, AM_ { am }
	{
	}

	void AzothSchemeHandler::requestStarted (QWebEngineUrlRequestJob *request)
	{
		const auto& url = request->requestUrl ();
		if (url.host () == "avatar"_ql)
		{
			LoadAvatar (url.path (), request);
			return;
		}

		qWarning () << Q_FUNC_INFO
				<< "unhandled request"
				<< url;
	}

	void AzothSchemeHandler::LoadAvatar (const QString& path, QWebEngineUrlRequestJob *request)
	{
		const auto handleImage = [request] (const QImage& image)
		{
			auto buffer = new QBuffer { request };
			buffer->open (QIODevice::WriteOnly);
			image.save (buffer, "PNG", 100);
			buffer->close ();

			request->reply ("image/x-png", buffer);
		};

		const auto& entryIdPath = path.section ('/', 1, 1).toLatin1 ();
		const auto& entryId = QString::fromUtf8 (QByteArray::fromBase64 (entryIdPath));

		const auto entryObj = Core::Instance ().GetEntry (entryId);
		if (!entryObj)
		{
			handleImage (ResourcesManager::Instance ().GetDefaultAvatar (64));
			return;
		}

		Util::Sequence (this, AM_->GetAvatar (entryObj, IHaveAvatars::Size::Thumbnail)) >>
				handleImage;
	}
}
