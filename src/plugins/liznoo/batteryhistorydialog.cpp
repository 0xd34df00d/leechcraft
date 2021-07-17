/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "batteryhistorydialog.h"
#include <algorithm>
#include <QPen>
#include <qwt_global.h>
#include <qwt_plot_curve.h>
#include <qwt_curve_fitter.h>
#include <qwt_legend.h>
#include <qwt_dyngrid_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_legenditem.h>
#include <util/util.h>
#include <util/gui/util.h>
#include "batteryinfo.h"

namespace LC
{
namespace Liznoo
{
	BatteryHistoryDialog::BatteryHistoryDialog (int histSize, double multiplier, QWidget *parent)
	: QDialog { parent }
	, Percent_ { new QwtPlotCurve }
	, Energy_ { new QwtPlotCurve }
	, Temperature_ { new QwtPlotCurve }
	, TimeMultiplier_ { multiplier }
	{
		Ui_.setupUi (this);

		Ui_.PercentPlot_->setAxisAutoScale (QwtPlot::xBottom, false);
		Ui_.PercentPlot_->setAxisAutoScale (QwtPlot::yLeft, false);
		Ui_.PercentPlot_->setAxisScale (QwtPlot::xBottom, 0, histSize * multiplier);
		Ui_.PercentPlot_->setAxisScale (QwtPlot::yLeft, 0, 100);
		Ui_.PercentPlot_->enableAxis (QwtPlot::yRight);

		InitNames ();

		QColor percentColor (Qt::blue);
		Percent_->setPen (QPen (percentColor));
		Util::TintPalette (Ui_.PercentPlot_->axisWidget (QwtPlot::yLeft), percentColor);
		percentColor.setAlpha (40);
		Percent_->setBrush (percentColor);

		Percent_->setRenderHint (QwtPlotItem::RenderAntialiased);
		Percent_->attach (Ui_.PercentPlot_);

		QColor energyColor (Qt::red);
		Energy_->setPen (QPen (energyColor));
		Util::TintPalette (Ui_.PercentPlot_->axisWidget (QwtPlot::yRight), energyColor);
		energyColor.setAlpha (40);
		Energy_->setBrush (energyColor);

		Energy_->setRenderHint (QwtPlotItem::RenderAntialiased);
		Energy_->setYAxis (QwtPlot::yRight);
		Energy_->attach (Ui_.PercentPlot_);

		QColor tempColor (Qt::green);
		Temperature_->setPen (QPen (tempColor));
		tempColor.setAlpha (20);
		Temperature_->setBrush (tempColor);

		Temperature_->setRenderHint (QwtPlotItem::RenderAntialiased);

		auto item = new QwtPlotLegendItem;
		item->setMaxColumns (1);
#if QWT_VERSION >= 0x060200
		item->setAlignmentInCanvas (Qt::AlignTop | Qt::AlignLeft);
#else
		item->setAlignment (Qt::AlignTop | Qt::AlignLeft);
#endif
		item->attach (Ui_.PercentPlot_);

		auto bgColor = palette ().color (QPalette::Button);
		bgColor.setAlphaF (0.8);
		item->setBackgroundBrush (bgColor);
		item->setBorderRadius (3);
		item->setBorderPen (QPen (palette ().color (QPalette::Dark), 1));
	}

	void BatteryHistoryDialog::UpdateHistory (const BatteryHistoryList& hist, const BatteryInfo& info)
	{
		QVector<double> xdata (hist.size ());
		QVector<double> energy (hist.size ());
		QVector<double> temperature (hist.size ());
		QVector<QPointF> percents;

		bool setTemperature = false;
		size_t i = 0;
		for (const auto& bh : hist)
		{
			if (percents.isEmpty () ||
					percents.last ().y () != bh.Percentage_ ||
					i == hist.size () - 1)
				percents.append ({ static_cast<qreal> (i * TimeMultiplier_), static_cast<qreal> (bh.Percentage_) });

			energy [i] = bh.EnergyRate_;

			temperature [i] = bh.Temperature_ - 273.15;
			setTemperature = bh.Temperature_ || setTemperature;

			xdata [i] = i * TimeMultiplier_;
			++i;
		}

		Percent_->setSamples (percents);
		Energy_->setSamples (xdata, energy);
		if (setTemperature)
		{
			Temperature_->attach (Ui_.PercentPlot_);
			Temperature_->setSamples (xdata, temperature);
		}

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

		const bool hasCyclesCount = info.CyclesCount_ > 0;
		Ui_.CyclesCount_->setVisible (hasCyclesCount);
		Ui_.CyclesCountLabel_->setVisible (hasCyclesCount);
		if (hasCyclesCount)
			Ui_.CyclesCount_->setText (QString::number (info.CyclesCount_));

		Ui_.PercentageLabel_->setText (QString::number (info.Percentage_) + "% " + chargeStateStr);
	}

	void BatteryHistoryDialog::InitNames ()
	{
		Percent_->setTitle (tr ("Percentage"));
		Energy_->setTitle (tr ("Energy rate"));
		Temperature_->setTitle (tr ("Temperature"));
		Ui_.PercentPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Charge, %"));
		Ui_.PercentPlot_->setAxisTitle (QwtPlot::yRight, tr ("Energy rate, W"));
		Ui_.PercentPlot_->setAxisTitle (QwtPlot::xBottom, tr ("Time, s"));
	}
}
}
