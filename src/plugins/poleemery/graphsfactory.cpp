/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "graphsfactory.h"
#include <numeric>
#include <QStringList>
#include <QMap>
#include <QApplication>
#include <qwt_plot_curve.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_tradingcurve.h>
#include <qwt_date_scale_draw.h>
#include <qwt_plot.h>
#include <util/sll/prelude.h>
#include "core.h"
#include "operationsmanager.h"
#include "accountsmanager.h"
#include "currenciesmanager.h"
#include "storage.h"

namespace LC
{
namespace Poleemery
{
	namespace
	{
		template<typename G, typename Iter>
		void FilterBySpan (const DateSpan_t& span, Iter& pos, Iter& lastPos, G dateGetter)
		{
			auto begin = pos, end = lastPos;
			pos = std::lower_bound (begin, end, span.first,
					[dateGetter] (const auto& entry, const QDateTime& dt) { return dateGetter (entry) < dt; });
			lastPos  = std::upper_bound (begin, end, span.second,
					[dateGetter] (const QDateTime& dt, const auto& entry) { return dt < dateGetter (entry); });
		}

		QMap<double, BalanceInfo> GetDays2Infos (const DateSpan_t& span)
		{
			auto opsMgr = Core::Instance ().GetOpsManager ();
			const auto& entries = opsMgr->GetEntriesWBalance ();

			auto pos = entries.begin (), lastPos = entries.end ();
			FilterBySpan (span, pos, lastPos,
					[] (const EntryWithBalance& entry) { return entry.Entry_->Date_; });

			if (pos == entries.end ())
				return {};

			const auto days = pos->Entry_->Date_.daysTo (span.second) + 1;

			QMap<double, BalanceInfo> days2infos;
			for (; pos != lastPos; ++pos)
			{
				const auto& then = pos->Entry_->Date_;

				const auto daysBack = then.daysTo (span.second);
				days2infos [(days - daysBack) + then.time ().hour () / 24.] = pos->Balance_;
			}

			if (days2infos.isEmpty ())
				return days2infos;

			auto minNum = days2infos.begin ().key ();
			auto prevBalance = days2infos.begin ().value ();
			for (int d = 0; d < days; ++d)
				for (int h = 0; h < 48; ++h)
				{
					const auto val = d + h / 48.;
					if (val <= minNum)
						continue;

					if (days2infos.contains (val))
					{
						prevBalance = days2infos.value (val);
						continue;
					}

					days2infos [val] = prevBalance;
				}

			return days2infos;
		}

		QMap<int, double> GetLastBalances (const QMap<double, BalanceInfo>& days2infos)
		{
			auto accsMgr = Core::Instance ().GetAccsManager ();
			const auto& accs = accsMgr->GetAccounts ();

			QMap<int, double> lastBalances;
			for (const auto& balance : days2infos)
			{
				for (const auto& id : balance.Accs_.keys ())
					if (!lastBalances.contains (id))
						lastBalances [id] = balance.Accs_ [id];

				if (accs.size () == lastBalances.size ())
					break;
			}

			return lastBalances;
		}

		void AddUp (QMap<int, QVector<double>>& accBalances)
		{
			if (accBalances.isEmpty ())
				return;

			QVector<double> curSum (accBalances.begin ()->size (), 0);
			for (auto& vec : accBalances)
				curSum = vec = Util::ZipWith (vec, curSum, std::plus<double> ());
		}

		QList<QColor> GenerateColors (int numColors)
		{
			QList<QColor> result;
			for (int i = 0; i < numColors; ++i)
			{
				QColor color;
				color.setHsvF (i / static_cast<double> (numColors), 0.8, 0.8);
				result << color;
			}
			return result;
		}

		QList<QwtPlotItem*> CreateBalanceItems (const DateSpan_t& span, bool cumulative)
		{
			const auto& days2infos = GetDays2Infos (span);
			const auto& xData = days2infos.keys ().toVector ();
			auto lastBalances = GetLastBalances (days2infos);

			const auto& periodAccounts = lastBalances.keys ();

			QMap<int, QVector<double>> accBalances;
			for (const auto& balance : days2infos)
				for (auto acc : periodAccounts)
				{
					auto value = balance.Accs_.value (acc, lastBalances [acc]);
					accBalances [acc] << value;
					lastBalances [acc] = value;
				}

			auto accsMgr = Core::Instance ().GetAccsManager ();
			auto curMgr = Core::Instance ().GetCurrenciesManager ();
			for (auto accId : accBalances.keys ())
			{
				const auto& acc = accsMgr->GetAccount (accId);
				const auto& rate = curMgr->GetUserCurrencyRate (acc.Currency_);
				if (rate != 1)
				{
					auto& vec = accBalances [accId];
					vec = Util::Map (vec, [rate] (double x) { return x * rate; });
				}
			}

			if (cumulative)
				AddUp (accBalances);

			const auto& colors = GenerateColors (periodAccounts.size ());
			int currentColor = 0;

			QList<QwtPlotItem*> result;
			int z = periodAccounts.size ();
			for (auto accId : periodAccounts)
			{
				const auto& acc = accsMgr->GetAccount (accId);

				auto curColor = colors.at (currentColor++ % colors.size ());

				auto item = new QwtPlotCurve (acc.Name_);
				item->setPen (curColor);

				if (!cumulative)
					curColor.setAlphaF (0.2);
				item->setBrush (curColor);

				item->setZ (z--);
				item->setSamples (xData, accBalances [accId]);

				result << item;
			}

			auto grid = new QwtPlotGrid;
			grid->enableYMin (true);
			grid->enableXMin (true);
			grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
			result << grid;

			return result;
		}

		QList<EntryBase_ptr> GetLastEntries (const DateSpan_t& span)
		{
			auto opsMgr = Core::Instance ().GetOpsManager ();
			auto entries = opsMgr->GetAllEntries ();

			std::sort (entries.begin (), entries.end (),
					[] (EntryBase_ptr l, EntryBase_ptr r)
					{
						return l->Date_ == r->Date_ ?
								l->Amount_ < r->Amount_ :
								l->Date_ < r->Date_;
					});

			for (auto i = entries.begin (); i < entries.end () - 1; )
			{
				const auto& item = *i;
				const auto& next = *(i + 1);
				if (item->Date_ == next->Date_ &&
						item->Amount_ == next->Amount_ &&
						item->GetType () != next->GetType ())
					i = entries.erase (i, i + 2);
				else
					++i;
			}

			auto pos = entries.begin (), lastPos = entries.end ();
			FilterBySpan (span, pos, lastPos,
					[] (const EntryBase_ptr& entry) { return entry->Date_; });
			entries.erase (entries.begin (), pos);
			if (lastPos < entries.end ())
				entries.erase (lastPos + 1, entries.end ());
			return entries;
		}

		QList<QwtPlotItem*> CreateSpendingBreakdownItems (const DateSpan_t& span, bool absolute)
		{
			double income = 0;
			double savings = 0;
			QMap<QString, double> cat2amount;

			auto accsMgr = Core::Instance ().GetAccsManager ();
			auto curMgr = Core::Instance ().GetCurrenciesManager ();
			for (auto entry : GetLastEntries (span))
			{
				auto acc = accsMgr->GetAccount (entry->AccountID_);
				const auto amount = curMgr->ToUserCurrency (acc.Currency_, entry->Amount_);

				switch (entry->GetType ())
				{
				case EntryType::Expense:
				{
					auto expense = std::dynamic_pointer_cast<ExpenseEntry> (entry);
					if (expense->Categories_.isEmpty ())
						cat2amount [QObject::tr ("uncategorized")] += amount;
					else
						for (const auto& cat : expense->Categories_)
							cat2amount [cat] += amount;

					savings -= amount;
					break;
				}
				case EntryType::Receipt:
					income += amount;
					savings += amount;
					break;
				}
			}

			if (income > 0)
			{
				if (absolute)
					cat2amount [QObject::tr ("income")] = income;
				if (savings > 0)
					cat2amount [QObject::tr ("savings")] = savings;
			}

			if (cat2amount.isEmpty ())
				return {};

			if (!absolute)
			{
				const auto sum = std::accumulate (cat2amount.begin (), cat2amount.end (), 0.0);
				for (auto& val : cat2amount)
					val = val * 100 / sum;
			}

			const auto& colors = GenerateColors (cat2amount.size ());
			int currentIndex = 0;

			QList<QwtPlotItem*> result;
			for (const auto& cat : cat2amount.keys ())
			{
				auto item = new QwtPlotHistogram (cat);

				auto curColor = colors.at (currentIndex++ % colors.size ());

				item->setPen (curColor);
				item->setBrush (curColor);
				item->setSamples ({ { cat2amount [cat],
							static_cast<double> (currentIndex * 5),
							static_cast<double> (currentIndex * 5 + 1) } });

				result << item;
			}

			auto grid = new QwtPlotGrid;
			grid->enableYMin (true);
			grid->enableX (false);
			grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
			result << grid;

			return result;
		}

		QList<QwtPlotItem*> CreateCurrenciesRatesItems (const DateSpan_t& span, const QString& currency, bool ohlc)
		{
			const auto& rates = Core::Instance ().GetStorage ()->GetRate (currency, span.first, span.second);

			QColor color (Qt::darkGreen);

			QList<QwtPlotItem*> result;
			if (ohlc)
			{
				QMap<int, QList<double>> byDay;
				for (const auto& rate : rates)
				{
					const auto days = span.first.date ().daysTo (rate.SnapshotTime_.date ());
					byDay [days] << rate.Rate_;
				}

				QVector<QwtOHLCSample> samples;
				for (auto i = byDay.begin (), end = byDay.end (); i != end; ++i)
				{
					const auto high = *std::max_element (i->begin (), i->end ());
					const auto low = *std::min_element (i->begin (), i->end ());

					const QDateTime datetime { span.first.date ().addDays (i.key ()), { 12, 00 } };
					const QwtOHLCSample sample (QwtDate::toDouble (datetime),
							i->first (), high, low, i->last ());
					samples << sample;
				}
				auto item = new QwtPlotTradingCurve (currency);
				item->setSamples (samples);
				item->setSymbolPen (color);
				item->setSymbolExtent (QwtDate::toDouble (QDateTime::fromSecsSinceEpoch (0).addDays (1)) * 3 / 5);
				result << item;
			}
			else
			{
				QVector<double> xData;
				QVector<double> yData;

				for (const auto& rate : rates)
				{
					xData << QwtDate::toDouble (rate.SnapshotTime_);
					yData << rate.Rate_;
				}

				auto item = new QwtPlotCurve (currency);
				item->setSamples (xData, yData);
				item->setPen (color);
				result << item;
			}

			auto grid = new QwtPlotGrid;
			grid->enableYMin (true);
			grid->enableXMin (true);
			grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
			result << grid;

			return result;
		}
	}

	GraphsFactory::GraphsFactory ()
	{
		auto prepareCummulative = [] (QwtPlot *plot) -> void
		{
			auto curMgr = Core::Instance ().GetCurrenciesManager ();
			plot->enableAxis (QwtPlot::Axis::xBottom, true);
			plot->enableAxis (QwtPlot::Axis::yLeft, true);
			plot->setAxisTitle (QwtPlot::Axis::xBottom, QObject::tr ("Days"));
			plot->setAxisTitle (QwtPlot::Axis::yLeft, curMgr->GetUserCurrency ());

			plot->setAxisScaleDraw (QwtPlot::Axis::xBottom, new QwtScaleDraw ());
		};

		Infos_.append ({
				QObject::tr ("Cumulative accounts balance"),
				[] (const DateSpan_t& span) { return CreateBalanceItems (span, true); },
				prepareCummulative
			});
		Infos_.append ({
				QObject::tr ("Comparative accounts balance"),
				[] (const DateSpan_t& span) { return CreateBalanceItems (span, false); },
				prepareCummulative
			});

		auto prepareRelBreakdown = [] (QwtPlot *plot)
		{
			plot->enableAxis (QwtPlot::Axis::xBottom, false);
			plot->enableAxis (QwtPlot::Axis::yLeft, true);
			plot->setAxisTitle (QwtPlot::Axis::yLeft, "%");

			plot->setAxisScaleDraw (QwtPlot::Axis::xBottom, new QwtScaleDraw ());
		};
		auto prepareAbsBreakdown = [] (QwtPlot *plot)
		{
			plot->enableAxis (QwtPlot::Axis::xBottom, false);
			plot->enableAxis (QwtPlot::Axis::yLeft, true);

			auto curMgr = Core::Instance ().GetCurrenciesManager ();
			plot->setAxisTitle (QwtPlot::Axis::yLeft, curMgr->GetUserCurrency ());

			plot->setAxisScaleDraw (QwtPlot::Axis::xBottom, new QwtDateScaleDraw ());
		};
		Infos_.append ({
				QObject::tr ("Per-category spendings breakdown (absolute)"),
				[] (const DateSpan_t& span) { return CreateSpendingBreakdownItems (span, true); },
				prepareAbsBreakdown
			});
		Infos_.append ({
				QObject::tr ("Per-category spendings breakdown (relative)"),
				[] (const DateSpan_t& span) { return CreateSpendingBreakdownItems (span, false); },
				prepareRelBreakdown
			});

		auto curMgr = Core::Instance ().GetCurrenciesManager ();
		for (const auto& cur : curMgr->GetEnabledCurrencies ())
		{
			if (cur == "USD")
				continue;

			Infos_.append ({
					QObject::tr ("%1 to USD rate (OHLC)").arg (cur),
					[cur] (const DateSpan_t& span) { return CreateCurrenciesRatesItems (span, cur, true); },
					[] (QwtPlot *plot) -> void
					{
						auto curMgr = Core::Instance ().GetCurrenciesManager ();
						plot->enableAxis (QwtPlot::Axis::xBottom, true);
						plot->enableAxis (QwtPlot::Axis::yLeft, true);
						plot->setAxisTitle (QwtPlot::Axis::xBottom, QObject::tr ("Days"));
						plot->setAxisTitle (QwtPlot::Axis::yLeft, curMgr->GetUserCurrency ());

						plot->setAxisScaleDraw (QwtPlot::Axis::xBottom, new QwtDateScaleDraw ());
					}
				});
			Infos_.append ({
					QObject::tr ("%1 to USD rate (standard curve)").arg (cur),
					[cur] (const DateSpan_t& span) { return CreateCurrenciesRatesItems (span, cur, false); },
					[] (QwtPlot *plot) -> void
					{
						auto curMgr = Core::Instance ().GetCurrenciesManager ();
						plot->enableAxis (QwtPlot::Axis::xBottom, true);
						plot->enableAxis (QwtPlot::Axis::yLeft, true);
						plot->setAxisTitle (QwtPlot::Axis::xBottom, QObject::tr ("Days"));
						plot->setAxisTitle (QwtPlot::Axis::yLeft, curMgr->GetUserCurrency ());

						plot->setAxisScaleDraw (QwtPlot::Axis::xBottom, new QwtDateScaleDraw ());
					}
				});
		}
	}

	QStringList GraphsFactory::GetNames () const
	{
		QStringList result;
		for (const auto& info : Infos_)
			result << info.Name_;
		return result;
	}

	QList<QwtPlotItem*> GraphsFactory::CreateItems (int index, const DateSpan_t& span)
	{
		if (index < 0 || index >= Infos_.size ())
			return {};

		return Infos_.at (index).Creator_ (span);
	}

	void GraphsFactory::PreparePlot (int index, QwtPlot *plot)
	{
		if (index < 0 || index >= Infos_.size ())
			return;

		Infos_.at (index).Preparer_ (plot);
	}
}
}
