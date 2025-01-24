/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "ui_batteryhistorydialog.h"
#include "batteryhistory.h"

class QwtPlotCurve;

namespace LC
{
namespace Liznoo
{
	class BatteryHistoryDialog : public QDialog
	{
		Q_OBJECT

		Ui::BatteryHistoryDialog Ui_;

		QwtPlotCurve *Percent_;
		QwtPlotCurve *Energy_;
		QwtPlotCurve *Temperature_;

		const double TimeMultiplier_;
	public:
		BatteryHistoryDialog (int size, double multiplier, QWidget* = 0);

		void UpdateHistory (const BatteryHistoryList&, const BatteryInfo&);
	private:
		void InitNames ();
	};
}
}
