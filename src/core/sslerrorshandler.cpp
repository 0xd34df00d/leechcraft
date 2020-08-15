/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorshandler.h"
#include <QSslError>
#include <QNetworkReply>
#include <QSettings>
#include <QCoreApplication>
#include <QPointer>
#include <QtDebug>
#include "sslerrorsdialog.h"

namespace LC
{
	namespace
	{
		void HandleFinished (int result,
				SslErrorsDialog::RememberChoice rc,
				const QString& urlString,
				const QString& host,
				const std::shared_ptr<QSettings>& settings)
		{
			const bool ignore = result == QDialog::Accepted;

			switch (rc)
			{
			case SslErrorsDialog::RememberChoice::File:
				settings->setValue (urlString, ignore);
				break;
			case SslErrorsDialog::RememberChoice::Host:
				settings->setValue (host, ignore);
				break;
			default:
				break;
			}
		}
	}

	SslErrorsHandler::SslErrorsHandler (QNetworkReply *reply,
			const QList<QSslError>& errors, QObject *parent)
	: QObject { parent }
	{
		const std::shared_ptr<QSettings> settings
		{
			new QSettings
			{
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName ()
			},
			[] (QSettings *settings)
			{
				settings->endGroup ();
				delete settings;
			}
		};
		settings->beginGroup ("SSL exceptions");
		const auto& keys = settings->allKeys ();
		const auto& url = reply->url ();
		const auto& urlString = url.toString ();
		const auto& host = url.host ();

		if (keys.contains (urlString))
		{
			if (settings->value (urlString).toBool ())
				reply->ignoreSslErrors ();

			return;
		}
		else if (keys.contains (host))
		{
			if (settings->value (host).toBool ())
				reply->ignoreSslErrors ();

			return;
		}

		auto errDialog = new SslErrorsDialog (urlString, errors);
		errDialog->setAttribute (Qt::WA_DeleteOnClose);
		errDialog->open ();

		const auto contSync = 0;
		const auto contAsync = 1;

		QEventLoop loop;
		connect (reply,
				&QNetworkReply::finished,
				this,
				[&loop, reply]
				{
					qDebug () << Q_FUNC_INFO
							<< "reply died, gonna exit"
							<< reply->error ();
					if (loop.isRunning ())
						loop.exit (contAsync);
				});
		connect (errDialog,
				&QDialog::finished,
				this,
				[&loop]
				{
					qDebug () << Q_FUNC_INFO
							<< "dialog accepted";
					if (loop.isRunning ())
						loop.exit (contSync);
				});

		const auto finishedHandler = [=]
				{
					HandleFinished (errDialog->result (), errDialog->GetRememberChoice (),
							urlString, host, settings);
				};

		if (loop.exec () == contAsync)
		{
			connect (errDialog,
					&QDialog::finished,
					this,
					finishedHandler);
			return;
		}
		finishedHandler ();

		switch (reply->error ())
		{
		case QNetworkReply::SslHandshakeFailedError:
			qWarning () << Q_FUNC_INFO
					<< "got SSL handshake error in handleSslErrors, but let's try to continue";
		case QNetworkReply::NoError:
			break;
		default:
			return;
		}

		if (errDialog->result () == QDialog::Accepted)
			reply->ignoreSslErrors ();
	}
}
