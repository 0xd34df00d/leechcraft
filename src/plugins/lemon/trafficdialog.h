/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QDialog>
#include "ui_trafficdialog.h"

class QwtPlotCurve;

namespace LC::Lemon
{
	class TrafficManager;

	class TrafficDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Lemon::TrafficDialog)

		Ui::TrafficDialog Ui_;

		TrafficManager *Manager_;
		const QString IfaceName_;

		QwtPlotCurve *DownTraffic_;
		QwtPlotCurve *UpTraffic_;

		QwtPlotCurve *DownAvg_;
		QwtPlotCurve *UpAvg_;
	public:
		TrafficDialog (const QString&, TrafficManager*, QWidget* = nullptr);
	private:
		void ReplotGraphs ();
	};
}
