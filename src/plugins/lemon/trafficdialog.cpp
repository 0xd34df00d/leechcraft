/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include "trafficmanager.h"

namespace LeechCraft
{
namespace Lemon
{
	TrafficDialog::TrafficDialog (const QString& name, TrafficManager *manager, QWidget *parent)
	: QDialog (parent)
	, Manager_ (manager)
	, IfaceName_ (name)
	, DownTraffic_ (new QwtPlotCurve (tr ("Down")))
	, UpTraffic_ (new QwtPlotCurve (tr ("Up")))
	{
		Ui_.setupUi (this);
		setWindowTitle (tr ("Traffic for %1").arg (name));

		Ui_.TrafficPlot_->setAxisScale (QwtPlot::xBottom, 0, manager->GetBacktrackSize ());
		Ui_.TrafficPlot_->setAxisTitle (QwtPlot::yLeft, tr ("Traffic, KiB/s"));

		QColor percentColor (Qt::blue);
		DownTraffic_->setPen (QPen (percentColor));
		percentColor.setAlpha (20);
		DownTraffic_->setBrush (percentColor);

		DownTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		DownTraffic_->attach (Ui_.TrafficPlot_);

		QColor energyColor (Qt::red);
		UpTraffic_->setPen (QPen (energyColor));
		energyColor.setAlpha (20);
		UpTraffic_->setBrush (energyColor);

		UpTraffic_->setRenderHint (QwtPlotItem::RenderAntialiased);
		UpTraffic_->attach (Ui_.TrafficPlot_);

		QwtLegend *legend = new QwtLegend;
		legend->setItemMode (QwtLegend::ClickableItem);
		Ui_.TrafficPlot_->insertLegend (legend, QwtPlot::BottomLegend);

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

		Ui_.TrafficPlot_->replot ();
	}
}
}
