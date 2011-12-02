/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "batteryhistorydialog.h"
#include <algorithm>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <qwt_legend.h>

namespace LeechCraft
{
namespace Liznoo
{
	BatteryHistoryDialog::BatteryHistoryDialog (int histSize, QWidget *parent)
	: QDialog (parent)
	, Percent_ (new QwtPlotCurve (tr ("Percentage")))
	, Energy_ (new QwtPlotCurve (tr ("Energy rate")))
	{
		Ui_.setupUi (this);

		Ui_.PercentPlot_->setAxisAutoScale (QwtPlot::xBottom, false);
		Ui_.PercentPlot_->setAxisAutoScale (QwtPlot::yLeft, false);
		Ui_.PercentPlot_->setAxisScale (QwtPlot::xBottom, 0, histSize);
		Ui_.PercentPlot_->setAxisScale (QwtPlot::yLeft, 0, 100);
		Ui_.PercentPlot_->enableAxis (QwtPlot::yRight);
		Ui_.PercentPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Percent"));
		Ui_.PercentPlot_->setAxisTitle (QwtPlot::yRight, tr ("Energy rate, W"));

		QColor percentColor (Qt::blue);
		Percent_->setPen (QPen (percentColor));
		percentColor.setAlpha (20);
		Percent_->setBrush (percentColor);

		Percent_->setRenderHint (QwtPlotItem::RenderAntialiased);
		Percent_->attach (Ui_.PercentPlot_);

		QColor energyColor (Qt::red);
		Energy_->setPen (QPen (energyColor));
		energyColor.setAlpha (20);
		Energy_->setBrush (energyColor);

		Energy_->setRenderHint (QwtPlotItem::RenderAntialiased);
		Energy_->setYAxis (QwtPlot::yRight);
		Energy_->attach (Ui_.PercentPlot_);
		
		QwtLegend *legend = new QwtLegend;
		legend->setItemMode (QwtLegend::ClickableItem);
		Ui_.PercentPlot_->insertLegend (legend, QwtPlot::BottomLegend);
	}

	void BatteryHistoryDialog::UpdateHistory (const QLinkedList<BatteryHistory>& hist)
	{
		QVector<double> xdata (hist.size ());
		QVector<double> percents (hist.size ());
		QVector<double> energy (hist.size ());
		
		int i = 0;
		std::for_each (hist.begin (), hist.end (),
				[&xdata, &percents, &energy, &i] (const BatteryHistory& bh)
				{
					percents [i] = bh.Percentage_;
					energy [i] = bh.EnergyRate_;
					xdata [i] = i++;
				});
		Percent_->setSamples (xdata, percents);
		Energy_->setSamples (xdata, energy);

		Ui_.PercentPlot_->replot ();
	}
}
}
