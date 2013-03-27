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

#include "batteryhistorydialog.h"
#include <algorithm>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <qwt_legend.h>
#include <qwt_dyngrid_layout.h>
#include "batteryinfo.h"
#include <util/util.h>

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
		Ui_.PercentPlot_->insertLegend (legend, QwtPlot::ExternalLegend);

		auto layout = qobject_cast<QwtDynGridLayout*> (legend->contentsWidget ()->layout ());
		if (layout)
			layout->setMaxCols (1);
		else
			qWarning () << Q_FUNC_INFO
					<< "legend contents layout is not a QwtDynGridLayout:"
					<< legend->contentsWidget ()->layout ();

		Ui_.InfoFrame_->layout ()->addWidget (legend);
	}

	void BatteryHistoryDialog::UpdateHistory (const QLinkedList<BatteryHistory>& hist, const BatteryInfo& info)
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
					xdata [i] = i;
					++i;
				});
		Percent_->setSamples (xdata, percents);
		Energy_->setSamples (xdata, energy);

		Ui_.PercentPlot_->replot ();

		QString chargeStateStr;
		if (info.TimeToEmpty_ && info.TimeToEmpty_ < 3600 * 24)
		{
			Ui_.RemainingTimeLabel_->setVisible (true);
			Ui_.RemainingTime_->setVisible (true);
			Ui_.RemainingTime_->setText (Util::MakeTimeFromLong (info.TimeToEmpty_));

			chargeStateStr = tr ("(discharging)");
		}
		else if (info.TimeToFull_ && info.TimeToFull_ < 3600 * 24)
		{
			Ui_.RemainingTimeLabel_->setVisible (true);
			Ui_.RemainingTime_->setVisible (true);
			Ui_.RemainingTime_->setText (Util::MakeTimeFromLong (info.TimeToFull_));

			chargeStateStr = tr ("(charging)");
		}
		else
		{
			Ui_.RemainingTimeLabel_->setVisible (false);
			Ui_.RemainingTime_->setVisible (false);
		}

		if (info.Temperature_ > 100)
		{
			Ui_.TempLabel_->setVisible (true);
			Ui_.Temp_->setVisible (true);
			Ui_.Temp_->setText (QString::fromUtf8 ("%1 °C").arg (info.Temperature_ - 273.15));
		}
		else
		{
			Ui_.TempLabel_->setVisible (false);
			Ui_.Temp_->setVisible (false);
		}

		if (info.Voltage_)
		{
			Ui_.VoltageLabel_->setVisible (true);
			Ui_.Voltage_->setVisible (true);
			Ui_.Voltage_->setText (tr ("%1 V").arg (info.Voltage_, 0, 'f', 3));
		}
		else
		{
			Ui_.VoltageLabel_->setVisible (false);
			Ui_.Voltage_->setVisible (false);
		}

		const bool energyAvailable = info.DesignEnergyFull_ > 1 && info.Energy_ > 1 && info.EnergyFull_ > 1;
		if (energyAvailable)
		{
			Ui_.DesignCapacity_->setText (tr ("%1 mAh").arg (info.DesignEnergyFull_, 0, 'f', 2));
			Ui_.LastFullCapacity_->setText (tr ("%1 mAh").arg (info.EnergyFull_, 0, 'f', 2));
			Ui_.Capacity_->setText (tr ("%1 mAh").arg (info.Energy_, 0, 'f', 2));

			const auto ratio = info.EnergyFull_ / info.DesignEnergyFull_;
			QString ratioText;
			if (ratio > 0.9)
				ratioText = tr ("awesome");
			else if (ratio > 0.7)
				ratioText = tr ("good");
			else if (ratio > 0.4)
				ratioText = tr ("degraded");
			else
				ratioText = tr ("bad");
			Ui_.Health_->setText (tr ("%1% (%2)")
					.arg (ratio * 100, 0, 'f', 1)
					.arg (ratioText));
		}
		Ui_.DesignCapacityLabel_->setVisible (energyAvailable);
		Ui_.DesignCapacity_->setVisible (energyAvailable);
		Ui_.LastFullCapacityLabel_->setVisible (energyAvailable);
		Ui_.LastFullCapacity_->setVisible (energyAvailable);
		Ui_.CapacityLabel_->setVisible (energyAvailable);
		Ui_.Capacity_->setVisible (energyAvailable);
		Ui_.HealthLabel_->setVisible (energyAvailable);
		Ui_.Health_->setVisible (energyAvailable);

		Ui_.PercentageLabel_->setText (QString::number (info.Percentage_) + "% " + chargeStateStr);
	}
}
}
