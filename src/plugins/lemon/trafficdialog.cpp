/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "trafficdialog.h"
#include <QtDebug>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_dyngrid_layout.h>
#include <util/util.h>
#include "trafficmanager.h"

namespace LeechCraft
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

		Ui_.TrafficPlot_->setAutoReplot (false);
		Ui_.TrafficPlot_->setAxisScale (QwtPlot::xBottom, 0, manager->GetBacktrackSize ());
		Ui_.TrafficPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Traffic, KiB/s"));

		QColor downColor (Qt::blue);
		DownTraffic_->setPen (QPen (downColor));
		downColor.setAlpha (20);
		DownTraffic_->setBrush (downColor);
		DownTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		DownTraffic_->attach (Ui_.TrafficPlot_);

		QColor upColor (Qt::red);
		UpTraffic_->setPen (QPen (upColor));
		upColor.setAlpha (20);
		UpTraffic_->setBrush (upColor);

		UpTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		UpTraffic_->attach (Ui_.TrafficPlot_);

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

		auto grid = new QwtPlotGrid;
		grid->enableYMin (true);
		grid->enableX (false);
		grid->setMinPen (QPen (Qt::gray, 1, Qt::DashLine));
		grid->attach (Ui_.TrafficPlot_);

		QwtLegend *legend = new QwtLegend;
		legend->setItemMode (QwtLegend::CheckableItem);
		Ui_.TrafficPlot_->insertLegend (legend, QwtPlot::ExternalLegend);

		auto layout = qobject_cast<QwtDynGridLayout*> (legend->contentsWidget ()->layout ());
		if (layout)
			layout->setMaxCols (1);
		else
			qWarning () << Q_FUNC_INFO
					<< "legend contents layout is not a QwtDynGridLayout:"
					<< legend->contentsWidget ()->layout ();

		Ui_.StatsFrame_->layout ()->addWidget (legend);

		connect (manager,
				SIGNAL (updated ()),
				this,
				SLOT (handleUpdated ()));
		handleUpdated ();
	}

	void TrafficDialog::handleUpdated ()
	{
		const auto& downList = Manager_->GetDownHistory (IfaceName_);
		const auto& upList = Manager_->GetUpHistory (IfaceName_);

		QVector<double> xdata (downList.size ());
		QVector<double> down (downList.size ());
		QVector<double> up (downList.size ());

		for (int i = 0; i < downList.size (); ++i)
		{
			xdata [i] = i;
			down [i] = downList [i] / 1024.;
			up [i] = upList [i] / 1024.;
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

			auto avgList = [] (const QList<qint64>& list)
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

	void TrafficDialog::on_TrafficPlot__legendChecked (QwtPlotItem *item, bool on)
	{
		item->setVisible (!on);
		Ui_.TrafficPlot_->replot ();
	}
}
}
