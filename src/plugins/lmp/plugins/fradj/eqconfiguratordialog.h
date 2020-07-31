/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_eqconfiguratordialog.h"
#include "bandinfo.h"

class QwtPlot;
class QwtPlotCurve;

namespace LC
{
namespace LMP
{
namespace Fradj
{
	class EqBandWidget;

	class EqConfiguratorDialog : public QDialog
	{
		Q_OBJECT

		Ui::EqConfiguratorDialog Ui_;

		QList<EqBandWidget*> Bands_;

		QwtPlot * const Plot_;
		QwtPlotCurve * const FreqCurve_;
	public:
		EqConfiguratorDialog (const BandInfos_t&,
				const QList<double>&,
				const QStringList& presets,
				QWidget* = 0);

		QList<double> GetGains () const;
		void SetGains (const QList<double>&);
	private:
		void SetupPlot ();
	private slots:
		void rebuildPlot ();
		void on_Preset__currentIndexChanged (const QString&);
	signals:
		void presetRequested (const QString&);
	};
}
}
}
