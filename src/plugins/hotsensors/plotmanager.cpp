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
#include <QStandardItemModel>
#include <QPainter>
#include <QSvgGenerator>
#include <QSvgRenderer>
#include <QBuffer>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>
#include "contextwrapper.h"
#include "sensorsgraphmodel.h"

namespace LeechCraft
{
namespace HotSensors
{
	PlotManager::PlotManager (std::weak_ptr<SensorsManager> mgr, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, SensorsMgr_ (mgr)
	, Model_ (new SensorsGraphModel (this))
	, UpdateCounter_ (0)
	{
	}

	QAbstractItemModel* PlotManager::GetModel () const
	{
		return Model_;
	}

	QObject* PlotManager::CreateContextWrapper ()
	{
		return new ContextWrapper (this, Proxy_);
	}

	void PlotManager::handleHistoryUpdated (const ReadingsHistory_t& history)
	{
		QHash<QString, QStandardItem*> existing;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto item = Model_->item (i);
			existing [item->data (SensorsGraphModel::SensorName).toString ()] = item;
		}

		QList<QStandardItem*> items;

		struct PendingSetInfo
		{
			QStandardItem *Item_;
			const QString Temp_;
			const QUrl URL_;
			const QString Name_;
			const QByteArray SVG_;
		};
		for (auto i = history.begin (); i != history.end (); ++i)
		{
			const auto& name = i.key ();

			QwtPlot plot;
			plot.setAxisAutoScale (QwtPlot::xBottom, false);
			plot.setAxisAutoScale (QwtPlot::yLeft, false);
			plot.enableAxis (QwtPlot::yLeft, false);
			plot.enableAxis (QwtPlot::xBottom, false);
			plot.resize (512, 512);
			plot.setAxisScale (QwtPlot::xBottom, 0, i->size ());
			plot.setAxisScale (QwtPlot::yLeft, 0, 100);
			plot.setAutoFillBackground (false);
			plot.setCanvasBackground (Qt::transparent);

			QwtPlotCurve curve;

			QColor percentColor ("#FF4B10");
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
			plot.replot ();

			QBuffer svgContents;
			svgContents.open (QIODevice::WriteOnly);

			QwtPlotRenderer renderer;
			QSvgGenerator gen;
			gen.setSize (plot.size ());
			gen.setViewBox (QRect { { 0, 0 }, plot.size () });
			gen.setOutputDevice (&svgContents);

			renderer.renderTo (&plot, gen);

			const bool isKnownSensor = existing.contains (name);
			auto item = isKnownSensor ? existing.take (name) : new QStandardItem;
			const QUrl url ("image://HS_sensorsGraph/" + name + "/" + QString::number (UpdateCounter_));

			const auto lastTemp = i->isEmpty () ? 0 : static_cast<int> (i->last ());
			item->setData (QString::fromUtf8 ("%1°C").arg (lastTemp), SensorsGraphModel::LastTemp);
			item->setData (url, SensorsGraphModel::IconURL);
			item->setData (name, SensorsGraphModel::SensorName);
			item->setData (svgContents.data (), SensorsGraphModel::SVG);
			if (!isKnownSensor)
				items << item;
		}

		for (auto item : existing)
			Model_->removeRow (item->row ());

		++UpdateCounter_;

		if (!items.isEmpty ())
			Model_->invisibleRootItem ()->appendRows (items);
	}
}
}
