/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QMap>

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Poleemery
{
	class CurrenciesManager : public QObject
	{
		Q_OBJECT

		QStringList Currencies_;
		QStandardItemModel *Model_;

		QStringList Enabled_;

		QMap<QString, double> RatesFromUSD_;

		QString UserCurrency_;

		QDateTime LastFetch_;
	public:
		CurrenciesManager (QObject* = 0);

		void Load ();

		const QStringList& GetEnabledCurrencies () const;
		QAbstractItemModel* GetSettingsModel () const;

		QString GetUserCurrency () const;
		double ToUserCurrency (const QString&, double) const;
		double GetUserCurrencyRate (const QString& from) const;
		double Convert (const QString& from, const QString& to, double value) const;
		double GetRate (const QString& from, const QString& to) const;
	private:
		void FetchRates (QStringList);
	private slots:
		void updateRates ();
		void gotRateReply ();
		void handleItemChanged (QStandardItem*);
	signals:
		void currenciesUpdated ();
	};
}
}
