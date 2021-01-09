/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorshandler.h"
#include <QTimer>
#include <QtDebug>
#include <QXmppClient.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	SslErrorsHandler::SslErrorsHandler (QXmppClient *client)
	: QObject { client }
	, Client_ { client }
	{
		Client_->configuration ().setIgnoreSslErrors (false);
		connect (Client_,
				SIGNAL (sslErrors (QList<QSslError>)),
				this,
				SLOT (handleSslErrors (QList<QSslError>)));
	}

	void SslErrorsHandler::EmitAborted ()
	{
		emit aborted ();
	}

	namespace
	{
		class SslErrorsReaction final : public ICanHaveSslErrors::ISslErrorsReaction
		{
			QXmppClient * const Client_;

			const QPointer<SslErrorsHandler> Handler_;
		public:
			SslErrorsReaction (QXmppClient *client, SslErrorsHandler *handler)
			: Client_ { client }
			, Handler_ { handler }
			{
			}

			void Ignore () override
			{
				qDebug () << Q_FUNC_INFO;

				Client_->configuration ().setIgnoreSslErrors (true);

				QTimer::singleShot (0, [client = Client_] { client->configuration ().setIgnoreSslErrors (false); });
			}

			void Abort () override
			{
				qDebug () << Q_FUNC_INFO;

				Client_->configuration ().setIgnoreSslErrors (false);
				if (Handler_)
					Handler_->EmitAborted ();
			}
		};
	}

	void SslErrorsHandler::handleSslErrors (const QList<QSslError>& errors)
	{
		emit sslErrors (errors, std::make_shared<SslErrorsReaction> (Client_, this));
	}
}
}
}
