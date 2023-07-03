/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lcschemehandler.h"
#include <QBuffer>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/poshuku/iinternalschemehandler.h>

namespace LC::Poshuku::WebEngineView
{
	namespace
	{
		auto GetScheme ()
		{
			QWebEngineUrlScheme scheme { "lc" };
			scheme.setSyntax (QWebEngineUrlScheme::Syntax::Path);
			scheme.setFlags (QWebEngineUrlScheme::SecureScheme |
					QWebEngineUrlScheme::LocalScheme |
					QWebEngineUrlScheme::LocalAccessAllowed);
			return scheme;
		}
	}

	void LcSchemeHandler::Register (QWebEngineProfile& profile)
	{
		profile.installUrlSchemeHandler (GetScheme ().name (), new LcSchemeHandler { &profile });
	}

	namespace
	{
		auto ToJobError (IInternalSchemeHandler::Error error)
		{
			switch (error)
			{
			case IInternalSchemeHandler::Error::Unsupported:
				return QWebEngineUrlRequestJob::NoError;
			case IInternalSchemeHandler::Error::NotFound:
				return QWebEngineUrlRequestJob::UrlNotFound;
			case IInternalSchemeHandler::Error::Denied:
				return QWebEngineUrlRequestJob::RequestDenied;
			}

			return QWebEngineUrlRequestJob::NoError;
		}

		auto ToJobDevice (QObject *context, const IInternalSchemeHandler::ReplyContents& contents)
		{
			return Util::Visit (contents,
					[context] (const QByteArray& bytes)
					{
						auto buffer = new QBuffer { context };
						buffer->setData (bytes);
						return static_cast<QIODevice*> (buffer);
					},
					[context] (const std::shared_ptr<QIODevice>& device)
					{
						// keep the device alive while the context is alive
						QObject::connect (context,
								&QObject::destroyed,
								context,
								[device] {});
						return device.get ();
					});
		}
	}

	void LcSchemeHandler::requestStarted (QWebEngineUrlRequestJob *job)
	{
		const auto& url = job->requestUrl ();
		if (url.scheme () != GetScheme ().name ())
			return;

		const auto& handlers = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IInternalSchemeHandler*> ();
		if (handlers.isEmpty ())
			return;

		const IInternalSchemeHandler::Request req
		{
			.Url_ = job->requestUrl (),
			.Initiator_ = job->initiator (),
		};
		for (const auto& handler : handlers)
		{
			const auto handled = Util::Visit (handler->HandleRequest (req),
					[job] (IInternalSchemeHandler::Error error)
					{
						switch (error)
						{
						case IInternalSchemeHandler::Error::Unsupported:
							return false;
						default:
							job->fail (ToJobError (error));
							return true;
						}
					},
					[job] (const IInternalSchemeHandler::Reply& reply)
					{
						job->reply (reply.ContentType_, ToJobDevice (job, reply.Contents_));
						return true;
					});

			if (handled)
				break;
		}
	}

	[[maybe_unused]] const auto LcSchemeRegistrar = []
	{
		auto scheme = GetScheme ();
		QWebEngineUrlScheme::registerScheme (scheme);
		return scheme;
	} ();
}
