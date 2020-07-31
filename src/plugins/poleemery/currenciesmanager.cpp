/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "currenciesmanager.h"
#include <cmath>
#include <limits>
#include <QLocale>
#include <QSet>
#include <QUrl>
#include <QStandardItemModel>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <QTimer>
#include <QDomDocument>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "structures.h"
#include "storage.h"

namespace LC
{
namespace Poleemery
{
	CurrenciesManager::CurrenciesManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	, UserCurrency_ (QLocale::system ().currencySymbol (QLocale::CurrencyIsoCode))
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Code"), tr ("Name") });
		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));
		Enabled_ = XmlSettingsManager::Instance ().property ("EnabledLocales").toStringList ();
		if (Enabled_.isEmpty ())
			Enabled_ << "USD" << UserCurrency_;
		Enabled_.sort ();

		RatesFromUSD_ ["USD"] = 1;
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poleemery");
		settings.beginGroup ("Currencies");
		LastFetch_ = settings.value ("LastFetch").toDateTime ();
		for (const auto& cur : settings.childKeys ())
			if (cur != "LastFetch")
				RatesFromUSD_ [cur] = settings.value (cur).toDouble ();
		settings.endGroup ();

		struct CurInfo
		{
			QString Code_;
			QString Name_;
		};
		QSet<QString> knownCodes;
		QList<CurInfo> currencies;
		for (auto language = 2; language < 214; ++language)
			for (auto country = 0; country < 247; ++country)
			{
				const QLocale loc (static_cast<QLocale::Language> (language),
						static_cast<QLocale::Country> (country));

				const auto& code = loc.currencySymbol (QLocale::CurrencyIsoCode);
				if (code.isEmpty ())
					continue;

				if (knownCodes.contains (code))
					continue;

				knownCodes << code;
				currencies.push_back ({ code, loc.currencySymbol (QLocale::CurrencyDisplayName) });
			}

		std::sort (currencies.begin (), currencies.end (),
				[] (const CurInfo& l, const CurInfo& r) { return l.Code_ < r.Code_; });

		for (const auto& cur : currencies)
		{
			Currencies_ << cur.Code_;

			QList<QStandardItem*> row { new QStandardItem (cur.Code_), new QStandardItem (cur.Name_) };
			for (auto item : row)
				item->setEditable (false);

			if (cur.Code_ != "USD")
			{
				row.first ()->setCheckable (true);
				row.first ()->setCheckState (Enabled_.contains (cur.Code_) ? Qt::Checked : Qt::Unchecked);
			}

			Model_->appendRow (row);
		}

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (updateRates ()));
		timer->start (10 * 60 * 1000);
	}

	void CurrenciesManager::Load ()
	{
		Enabled_.sort ();

		if (Enabled_ != RatesFromUSD_.keys () ||
				LastFetch_.msecsTo (QDateTime::currentDateTime ()) > 60 * 1000)
			updateRates ();
	}

	const QStringList& CurrenciesManager::GetEnabledCurrencies () const
	{
		return Enabled_;
	}

	QAbstractItemModel* CurrenciesManager::GetSettingsModel () const
	{
		return Model_;
	}

	QString CurrenciesManager::GetUserCurrency () const
	{
		return UserCurrency_;
	}

	double CurrenciesManager::ToUserCurrency (const QString& code, double value) const
	{
		return value * GetUserCurrencyRate (code);
	}

	double CurrenciesManager::GetUserCurrencyRate (const QString& code) const
	{
		if (code == UserCurrency_)
			return 1;

		return RatesFromUSD_.value (UserCurrency_, 1) / RatesFromUSD_.value (code, 1);
	}

	double CurrenciesManager::Convert (const QString& from, const QString& to, double value) const
	{
		if (from == to)
			return value;

		if (!RatesFromUSD_.contains (to))
			qWarning () << Q_FUNC_INFO
					<< "unknown target currency"
					<< to;
		if (!RatesFromUSD_.contains (from))
			qWarning () << Q_FUNC_INFO
					<< "unknown source currency"
					<< from;

		return value * RatesFromUSD_.value (to, 1) / RatesFromUSD_.value (from, 1);
	}

	double CurrenciesManager::GetRate (const QString& from, const QString& to) const
	{
		return Convert (from, to, 1);
	}

	void CurrenciesManager::FetchRates (QStringList values)
	{
		values.removeAll ("USD");
		QStringList subqueries;
		for (const auto& value : values)
			subqueries << "pair in (\"USD" + value + "\")";

		QString urlStr = QString ("http://query.yahooapis.com/v1/public/yql?q="
				"select * from yahoo.finance.xchange where %1&env=http://datatables.org/alltables.env").arg (subqueries.join (" or "));
		QUrl url (urlStr);

		auto nam = Core::Instance ().GetCoreProxy ()->GetNetworkAccessManager ();
		auto reply = nam->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (gotRateReply ()));
	}

	void CurrenciesManager::updateRates ()
	{
		FetchRates (Enabled_);
	}

	void CurrenciesManager::gotRateReply ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< data;
			return;
		}

		const auto& now = QDateTime::currentDateTime ();

		bool changed = false;
		auto rateElem = doc.documentElement ()
				.firstChildElement ("results")
				.firstChildElement ("rate");
		while (!rateElem.isNull ())
		{
			std::shared_ptr<void> guard (nullptr,
					[&rateElem] (void*) { rateElem = rateElem.nextSiblingElement ("rate"); });

			const auto& toValue = rateElem.attribute ("id").mid (3);
			if (toValue.size () != 3)
			{
				qWarning () << "incorrect `to` value"
						<< toValue;
				continue;
			}

			const auto newRate = rateElem.firstChildElement ("Rate").text ().toDouble ();
			if (std::fabs (newRate - RatesFromUSD_ [toValue]) > std::numeric_limits<double>::epsilon ())
			{
				RatesFromUSD_ [toValue] = newRate;
				changed = true;
			}

			Rate rate { 0, toValue, now, newRate };
			Core::Instance ().GetStorage ()->AddRate (rate);
		}

		LastFetch_ = QDateTime::currentDateTime ();

		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poleemery");
		settings.beginGroup ("Currencies");
		settings.setValue ("LastFetch", LastFetch_);
		if (changed)
		{
			emit currenciesUpdated ();
			for (auto i = RatesFromUSD_.constBegin (); i != RatesFromUSD_.constEnd (); ++i)
				settings.setValue (i.key (), *i);
		}
		settings.endGroup ();
	}

	void CurrenciesManager::handleItemChanged (QStandardItem *item)
	{
		if (item->column ())
			return;

		QStringList news;

		const auto& code = item->text ();
		if (item->checkState () == Qt::Unchecked)
			Enabled_.removeAll (code);
		else if (!Enabled_.contains (code))
		{
			news << code;
			Enabled_ << code;
			Enabled_.sort ();
		}
		FetchRates (news);

		XmlSettingsManager::Instance ().setProperty ("EnabledLocales", Enabled_);
	}
}
}
