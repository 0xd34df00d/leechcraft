/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eqconfiguratordialog.h"
#include <QtDebug>
#include <QPen>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_engine.h>
#include <qwt_curve_fitter.h>
#include <util/sll/prelude.h>
#include "eqbandwidget.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	EqConfiguratorDialog::EqConfiguratorDialog (const BandInfos_t& bands,
			const QList<double>& gains, const QStringList& presets, QWidget *parent)
	: QDialog { parent }
	, Plot_ { new QwtPlot }
	, FreqCurve_ { new QwtPlotCurve }
	{
		Ui_.setupUi (this);
		Ui_.Preset_->addItems (presets);
		Ui_.Preset_->setCurrentIndex (-1);

		for (const auto& bandInfo : bands)
		{
			auto band = new EqBandWidget { bandInfo };
			Ui_.BandsLayout_->addWidget (band);
			Bands_ << band;

			connect (band,
					SIGNAL (valueChanged (double)),
					this,
					SLOT (rebuildPlot ()));
		}

		SetupPlot ();

		SetGains (gains);
	}

	QList<double> EqConfiguratorDialog::GetGains () const
	{
		return Util::Map (Bands_, &EqBandWidget::GetGain);
	}

	void EqConfiguratorDialog::SetGains (const QList<double>& gains)
	{
		if (gains.size () != Bands_.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "gains count"
					<< gains.size ()
					<< "doesn't equal to bands count"
					<< Bands_.size ();
			return;
		}

		for (int i = 0; i < gains.size (); ++i)
			Bands_.at (i)->SetGain (gains.at (i));

		rebuildPlot ();
	}

	void EqConfiguratorDialog::SetupPlot ()
	{
		Ui_.DialogLayout_->insertWidget (Ui_.DialogLayout_->count () - 1, Plot_);

		Plot_->setAxisTitle (QwtPlot::xBottom, tr ("Frequency, Hz"));
		Plot_->setAxisScaleEngine (QwtPlot::xBottom, new QwtLogScaleEngine { 2 });

		Plot_->setAxisAutoScale (QwtPlot::yLeft, false);
		Plot_->setAxisScale (QwtPlot::yLeft, -24, 12);
		Plot_->setAxisTitle (QwtPlot::yLeft, tr ("Gain, dB"));

		FreqCurve_->setRenderHint (QwtPlotItem::RenderAntialiased);

		FreqCurve_->setCurveAttribute (QwtPlotCurve::Fitted);
		auto fitter = new QwtSplineCurveFitter;
		FreqCurve_->setCurveFitter (fitter);

		FreqCurve_->attach (Plot_);

		auto grid = new QwtPlotGrid;
		grid->enableXMin (true);
		grid->setMajorPen (QPen (Qt::gray, 1, Qt::DashLine));
		grid->setMinorPen (QPen (Qt::gray, 1, Qt::DashLine));
		grid->attach (Plot_);
	}

	void EqConfiguratorDialog::rebuildPlot ()
	{
		QVector<double> xData, yData;
		for (const auto& band : Bands_)
		{
			xData << band->GetFrequency ();
			yData << band->GetGain ();
		}
		Plot_->setAxisScale (QwtPlot::xBottom, xData.first (), xData.last ());
		FreqCurve_->setSamples (xData, yData);

		Plot_->replot ();
	}

	void EqConfiguratorDialog::on_Preset__currentIndexChanged (const QString& preset)
	{
		emit presetRequested (preset);
	}
}
}
}
