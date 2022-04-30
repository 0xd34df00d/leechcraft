/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trafficdialog.h"
#include <numeric>
#include <QtDebug>
#include <QPen>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_dyngrid_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_legenditem.h>
#include <util/util.h>
#include <util/gui/util.h>
#include "trafficmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Lemon
{
	TrafficDialog::TrafficDialog (const QString& name, TrafficManager *manager, QWidget *parent)
	: QDialog (parent)
	, Manager_ (manager)
	, IfaceName_ (name)
	, DownTraffic_ (new QwtPlotCurve (tr ("RX")))
	, UpTraffic_ (new QwtPlotCurve (tr ("TX")))
	, DownAvg_ (new QwtPlotCurve (tr ("Average RX")))
	, UpAvg_ (new QwtPlotCurve (tr ("Average TX")))
	{
		Ui_.setupUi (this);
		setWindowTitle (tr ("Traffic for %1").arg (name));

		const bool splitAxes = XmlSettingsManager::Instance ().property ("SplitAxes").toBool ();

		auto downColor = XmlSettingsManager::Instance ().property ("DownloadColor").value<QColor> ();
		auto upColor = XmlSettingsManager::Instance ().property ("UploadColor").value<QColor> ();

		Ui_.TrafficPlot_->setAutoReplot (false);
		Ui_.TrafficPlot_->setAxisScale (QwtPlot::xBottom, 0, manager->GetBacktrackSize ());
		if (splitAxes)
		{
			Ui_.TrafficPlot_->enableAxis (QwtPlot::yRight);
			Ui_.TrafficPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Download, KiB/s"));
			Ui_.TrafficPlot_->setAxisTitle (QwtPlot::yRight, tr ("Upload, KiB/s"));

			Util::TintPalette (Ui_.TrafficPlot_->axisWidget (QwtPlot::yLeft), downColor);
			Util::TintPalette (Ui_.TrafficPlot_->axisWidget (QwtPlot::yRight), upColor);
		}
		else
			Ui_.TrafficPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Traffic, KiB/s"));

		DownTraffic_->setPen (QPen (downColor));
		downColor.setAlpha (20);
		DownTraffic_->setBrush (downColor);
		DownTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		DownTraffic_->attach (Ui_.TrafficPlot_);

		UpTraffic_->setPen (QPen (upColor));
		upColor.setAlpha (20);
		UpTraffic_->setBrush (upColor);

		UpTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		UpTraffic_->attach (Ui_.TrafficPlot_);
		if (splitAxes)
			UpTraffic_->setYAxis (QwtPlot::yRight);

		downColor.setAlpha (100);
		DownAvg_->setPen (QPen (downColor, 2, Qt::DotLine));
		DownAvg_->setBrush (Qt::transparent);
		DownAvg_->setRenderHint (QwtPlotItem::RenderAntialiased, false);
		DownAvg_->attach (Ui_.TrafficPlot_);

		upColor.setAlpha (100);
		UpAvg_->setPen (QPen (upColor, 2, Qt::DotLine));
		UpAvg_->setBrush (Qt::transparent);
		UpAvg_->setRenderHint (QwtPlotItem::RenderAntialiased, false);
		UpAvg_->attach (Ui_.TrafficPlot_);
		if (splitAxes)
			UpAvg_->setYAxis (QwtPlot::yRight);

		auto grid = new QwtPlotGrid;
		grid->enableYMin (true);
		grid->enableX (false);
		grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
		grid->attach (Ui_.TrafficPlot_);

		auto item = new QwtPlotLegendItem;
		item->setMaxColumns (1);
#if QWT_VERSION >= 0x060200
		item->setAlignmentInCanvas (Qt::AlignTop | Qt::AlignLeft);
#else
		item->setAlignment (Qt::AlignTop | Qt::AlignLeft);
#endif
		item->attach (Ui_.TrafficPlot_);

		auto bgColor = palette ().color (QPalette::Button);
		bgColor.setAlphaF (0.8);
		item->setBackgroundBrush (bgColor);
		item->setBorderRadius (3);
		item->setBorderPen (QPen (palette ().color (QPalette::Dark), 1));

		connect (manager,
				SIGNAL (updated ()),
				this,
				SLOT (handleUpdated ()));
		handleUpdated ();

		Ui_.Legend_->setDefaultItemMode (QwtLegendData::Checkable);
		Ui_.Legend_->setMaxColumns (2);
		connect (Ui_.TrafficPlot_,
				&QwtPlot::legendDataChanged,
				Ui_.Legend_,
				&QwtLegend::updateLegend);
		Ui_.TrafficPlot_->updateLegend ();

		connect (Ui_.Legend_,
				&QwtLegend::checked,
				this,
				[this] (const QVariant& itemVar, bool on)
				{
					if (const auto item = itemVar.value<QwtPlotItem*> ())
					{
						item->setVisible (!on);
						Ui_.TrafficPlot_->replot ();
					}
				});
	}

	namespace
	{
		double GetSmoothed (const QVector<qint64>& list, size_t pos, bool smooth)
		{
			if (!smooth)
				return list.at (pos);

			const std::array<double, 3> kernel { { 1, 1.2, 1 } };
			const auto kernelSum = std::accumulate (kernel.begin (), kernel.end (), 0);

			if (static_cast<size_t> (list.size ()) < kernel.size ())
				return list.at (pos);

			const auto halfSize = kernel.size () / 2;
			if (pos < halfSize)
				return list.at (pos);

			double result = 0;
			for (size_t i = 0; i < kernel.size (); ++i)
			{
				auto listPos = i + pos - halfSize;
				if (listPos >= static_cast<size_t> (list.size ()))
					listPos = 2 * static_cast<size_t> (list.size ()) - listPos - 1;
				result += kernel.at (i) * list.at (listPos);
			}
			return result / kernelSum;
		}
	}

	void TrafficDialog::handleUpdated ()
	{
		const auto& downList = Manager_->GetDownHistory (IfaceName_);
		const auto& upList = Manager_->GetUpHistory (IfaceName_);

		QVector<double> xdata (downList.size ());
		QVector<double> down (downList.size ());
		QVector<double> up (downList.size ());

		const auto shouldSmooth = XmlSettingsManager::Instance ()
				.property ("EnableSmoothing").toBool ();

		for (int i = 0; i < downList.size (); ++i)
		{
			xdata [i] = i;
			down [i] = GetSmoothed (downList, i, shouldSmooth) / 1024.;
			up [i] = GetSmoothed (upList, i, shouldSmooth) / 1024.;
		}

		DownTraffic_->setSamples (xdata, down);
		UpTraffic_->setSamples (xdata, up);

		if (!downList.isEmpty ())
		{
			Ui_.StatsFrame_->setVisible (true);

			Ui_.RXSpeed_->setText (Util::MakePrettySize (downList.last ()) + tr ("/s"));
			Ui_.TXSpeed_->setText (Util::MakePrettySize (upList.last ()) + tr ("/s"));

			const auto maxRx = *std::max_element (downList.begin (), downList.end ());
			const auto maxTx = *std::max_element (upList.begin (), upList.end ());
			Ui_.MaxRXSpeed_->setText (Util::MakePrettySize (maxRx) + tr ("/s"));
			Ui_.MaxTXSpeed_->setText (Util::MakePrettySize (maxTx) + tr ("/s"));

			auto avgList = [] (const QVector<qint64>& list)
				{ return std::accumulate (list.begin (), list.end (), 0.0) / list.size (); };
			const auto avgRx = avgList (downList);
			const auto avgTx = avgList (upList);

			Ui_.AvgRXSpeed_->setText (Util::MakePrettySize (avgRx) + tr ("/s"));
			Ui_.AvgTXSpeed_->setText (Util::MakePrettySize (avgTx) + tr ("/s"));

			DownAvg_->setSamples (xdata, QVector<double> (downList.size (), avgRx / 1024));
			UpAvg_->setSamples (xdata, QVector<double> (downList.size (), avgTx / 1024));
		}
		else
			Ui_.StatsFrame_->setVisible (false);

		Ui_.TrafficPlot_->replot ();
	}
}
}
