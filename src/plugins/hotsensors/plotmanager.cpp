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

#include "plotmanager.h"
#include <QSvgGenerator>
#include <QBuffer>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>

namespace LeechCraft
{
namespace HotSensors
{
	PlotManager::PlotManager (std::weak_ptr<SensorsManager> mgr, QObject *parent)
	: QObject (parent)
	, SensorsMgr_ (mgr)
	{
	}

	void PlotManager::handleHistoryUpdated (const ReadingsHistory_t& history)
	{
		for (auto i = history.begin (); i != history.end (); ++i)
		{
			const auto& name = i.key ();

			QwtPlot plot;
			plot.enableAxis (QwtPlot::yLeft, false);
			plot.enableAxis (QwtPlot::xBottom, false);
			plot.resize (32, 32);

			QwtPlotCurve curve;

			QColor percentColor (Qt::blue);
			curve.setPen (QPen (percentColor));
			percentColor.setAlpha (20);
			curve.setBrush (percentColor);

			curve.setRenderHint (QwtPlotItem::RenderAntialiased);
			curve.attach (&plot);

			QVector<double> ySamples;
			QVector<double> xSamples;
			for (const auto& item : *i)
			{
				ySamples << item;
				xSamples << xSamples.size ();
			}

			curve.setSamples (xSamples, ySamples);

			QBuffer svgContents;
			svgContents.open (QIODevice::WriteOnly);

			QwtPlotRenderer renderer;
			QSvgGenerator gen;
			gen.setSize (plot.size ());
			gen.setViewBox (QRect { { 0, 0 }, plot.size () });
			gen.setOutputDevice (&svgContents);

			renderer.renderTo (&plot, gen);
		}
	}
}
}
