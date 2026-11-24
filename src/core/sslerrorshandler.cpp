/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorshandler.h"
#include <QCoreApplication>
#include <QNetworkReply>
#include <QSettings>
#include <QSslError>
#include <util/sll/util.h>
#include <util/threads/coro/context.h>
#include <util/threads/coro/signal.h>
#include "sslerrorsdialog.h"

namespace LC
{
	namespace
	{
		struct StoredDecisions : QSettings
		{
			StoredDecisions ()
			: QSettings { QCoreApplication::organizationName (), QCoreApplication::applicationName () }
			{
				beginGroup ("SSL exceptions");
			}
		};
	}

	void SslErrorsHandler::Handle (QNetworkReply *reply, const QList<QSslError>& errors)
	{
		const auto& host = reply->url ().host ();

		auto decision = SessionDecisions_.value (host, Decision::Undecided);
		if (const StoredDecisions stored;
			stored.contains (host))
			decision = stored.value (host).toBool () ? Decision::Accepted : Decision::Rejected;

		switch (decision)
		{
		case Decision::Accepted:
			reply->ignoreSslErrors (errors);
			break;
		case Decision::Rejected:
			break;
		case Decision::Undecided:
			if (!OpenQuestions_.contains (host))
				Ask (host, errors);
			break;
		}
	}

	Util::ContextTask<void> SslErrorsHandler::Ask (QString host, QList<QSslError> errors)
	{
		co_await Util::AddContext { CoroContext_ };

		OpenQuestions_ << host;
		const auto guard = Util::MakeScopeGuard ([this, host] { OpenQuestions_.remove (host); });

		SslErrorsDialog dia { host, errors };
		dia.show ();
		co_await Util::Signal { dia, &QDialog::finished };

		const bool accepted = dia.result () == QDialog::Accepted;
		if (dia.IsPermanent ())
			StoredDecisions {}.setValue (host, accepted);
		else
			SessionDecisions_ [host] = accepted ? Decision::Accepted : Decision::Rejected;
	}
}
