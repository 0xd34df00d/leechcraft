/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "favoriteschecker.h"
#include <QProgressDialog>
#include <QMessageBox>
#include <QApplication>
#include <QFontMetrics>
#include <QMainWindow>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	FavoritesChecker::FavoritesChecker (QObject *parent)
	: QObject (parent)
	, Model_ (Core::Instance ().GetFavoritesModel ())
	{
		ProgressDialog_ = new QProgressDialog (tr ("Checking Favorites..."),
				tr ("Cancel"),
				0, 0);
		connect (ProgressDialog_,
				SIGNAL (canceled ()),
				this,
				SLOT (handleCanceled ()));
	}

	void FavoritesChecker::Check ()
	{
		Items_ = Model_->GetItems ();

		for (const auto& item : Items_)
		{
			QUrl url = QUrl (item.URL_);
			QNetworkRequest req (url);
			QString ua = Core::Instance ().GetUserAgent (url);
			if (!ua.isEmpty ())
				req.setRawHeader ("User-Agent", ua.toLatin1 ());

			QNetworkReply *rep = Core::Instance ()
				.GetNetworkAccessManager ()->head (req);

			rep->setProperty ("SourceURL", url);

			connect (rep,
					SIGNAL (finished ()),
					this,
					SLOT (handleFinished ()));

			Pending_ << rep;
		}

		if (Pending_.isEmpty ())
		{
			deleteLater ();
			return;
		}

		ProgressDialog_->setRange (0, Pending_.size ());
		ProgressDialog_->setValue (0);
		ProgressDialog_->show ();
	}

	namespace
	{
		QString BuildMessage (const QStringList& list, const QString& property, int num)
		{
			QString result;
			if (!list.size ())
				result = "";
			else if (list.size () < num)
			{
				QString merged;
				QFontMetricsF fm { QApplication::font () };
				for (const auto& url : list)
					merged.append (QString ("<li>%1</li>").arg (fm.elidedText (url, Qt::ElideMiddle, 400)));
				result = FavoritesChecker::tr ("%1 favorites are %2:<br /><ul>%3</ul>")
					.arg (list.size ())
					.arg (property)
					.arg (merged);
			}
			else
				result = FavoritesChecker::tr ("%1 favorites are %2.<br />")
					.arg (list.size ())
					.arg (property);

			return result;
		}
	}

	void FavoritesChecker::HandleAllDone ()
	{
		ProgressDialog_->setValue (ProgressDialog_->value () + 1);

		int accessible = 0,
			serverStuff = 0;
		QStringList unaccessibleList;
		QStringList redirectsList;

		QMap<QString, QString> result;

		for (const auto& pair : Util::Stlize (Results_))
		{
			const auto& key = pair.first;
			const auto& res = pair.second;

			QString mres;
			if (res.Error_ != QNetworkReply::NoError)
			{
				unaccessibleList << key.toString ();
				mres = res.ErrorString_;
			}
			else if (res.StatusCode_ < 200 ||
					res.StatusCode_ > 399)
			{
				++serverStuff;
				mres = QString ("HTTP %1")
					.arg (res.StatusCode_);
			}
			else
			{
				++accessible;
				mres = tr ("HTTP %1")
					.arg (res.StatusCode_);
				if (res.Length_)
					mres += tr ("<br />Length: %1")
						.arg (res.Length_);
				if (res.LastModified_.isValid ())
					mres += tr ("<br />Last-modified: %1")
						.arg (res.LastModified_.toString ());

				if (res.RedirectURL_.isValid ())
				{
					redirectsList << key.toString ();
					mres += tr ("<br />Redirects to %1")
						.arg (res.RedirectURL_.toString ());
				}
			}

			result [key.toString ()] = mres;
		}

		Model_->SetCheckResults (result);

		QString message = tr ("%1 favorites total.<br />"
				"%2 favorites are accessible.<br />"
				"%3"
				"%4 are not correctly returned by the remote server.<br />"
				"%5")
			.arg (accessible + unaccessibleList.size () + serverStuff)
			.arg (accessible)
			.arg (BuildMessage (unaccessibleList, "unaccessible", 10))
			.arg (serverStuff)
			.arg (BuildMessage (redirectsList, "redirected", 10));

		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		QMessageBox::information (rootWM->GetPreferredWindow (),
				"LeechCraft",
				message);

		ProgressDialog_->reset ();

		deleteLater ();
	}

	void FavoritesChecker::handleFinished ()
	{
		QNetworkReply *rep = qobject_cast<QNetworkReply*> (sender ());
		if (!rep)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QNetworkReply*"
				<< sender ();
			return;
		}

		Pending_.removeAll (rep);
		rep->deleteLater ();

		QUrl url = rep->property ("SourceURL").value<QUrl> ();
		Result result =
		{
			rep->error (),
			rep->errorString (),
			rep->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt (),
			rep->attribute (QNetworkRequest::RedirectionTargetAttribute).value<QUrl> (),
			rep->header (QNetworkRequest::LastModifiedHeader).toDateTime (),
			rep->header (QNetworkRequest::ContentLengthHeader).value<qint64> ()
		};

		Results_ [url] = result;

		ProgressDialog_->setValue (ProgressDialog_->value () + 1);

		if (Pending_.isEmpty ())
			HandleAllDone ();
	}

	void FavoritesChecker::handleCanceled ()
	{
		qDeleteAll (Pending_);
		deleteLater ();
	}
}
}
