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
#include <QDeclarativeImageProvider>
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

namespace LeechCraft
{
namespace HotSensors
{
	namespace
	{
		class SensorsGraphModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				IconURL = Qt::UserRole + 1,
				LastTemp
			};

			SensorsGraphModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [IconURL] = "iconURL";
				roleNames [LastTemp] = "lastTemp";
				setRoleNames (roleNames);
			}
		};
	}

	class SensorsGraphProvider : public QDeclarativeImageProvider
	{
		QHash<QString, QByteArray> Sensor2SVG_;
	public:
		SensorsGraphProvider ()
		: QDeclarativeImageProvider (QDeclarativeImageProvider::Image)
		{
		}

		void SetHistory (const QHash<QString, QByteArray>& svg)
		{
			Sensor2SVG_ = svg;
		}

		QImage requestImage (const QString& id, QSize* size, const QSize& requestedSize)
		{
			const auto lastSlash = id.lastIndexOf ('/');
			const auto& sensorName = id.left (lastSlash);

			QImage result (requestedSize, QImage::Format_ARGB32);
			QPainter p (&result);
			QSvgRenderer renderer (Sensor2SVG_ [sensorName]);
			renderer.render (&p);
			p.end ();

			return result;
		}
	};

	PlotManager::PlotManager (std::weak_ptr<SensorsManager> mgr, QObject *parent)
	: QObject (parent)
	, SensorsMgr_ (mgr)
	, Model_ (new SensorsGraphModel (this))
	, GraphProvider_ (new SensorsGraphProvider)
	, UpdateCounter_ (0)
	{
	}

	QAbstractItemModel* PlotManager::GetModel () const
	{
		return Model_;
	}

	QDeclarativeImageProvider* PlotManager::GetImageProvider () const
	{
		return GraphProvider_;
	}

	void PlotManager::handleHistoryUpdated (const ReadingsHistory_t& history)
	{
		Model_->clear ();

		QList<QStandardItem*> items;
		QHash<QString, QByteArray> svg;
		for (auto i = history.begin (); i != history.end (); ++i)
		{
			const auto& name = i.key ();

			QwtPlot plot;
			plot.setAxisAutoScale (QwtPlot::xBottom, false);
			plot.setAxisAutoScale (QwtPlot::yLeft, false);
			plot.enableAxis (QwtPlot::yLeft, false);
			plot.enableAxis (QwtPlot::xBottom, false);
			plot.resize (128, 128);
			plot.setAxisScale (QwtPlot::xBottom, 0, i->size ());
			plot.setAxisScale (QwtPlot::yLeft, 0, 100);

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

			auto item = new QStandardItem;
			const QUrl url ("image://HS_sensorsGraph/" + name + "/" + QString::number (UpdateCounter_));

			const auto lastTemp = i->isEmpty () ? 0 : static_cast<int> (i->last ());
			item->setData (QString::fromUtf8 ("%1°C").arg (lastTemp), SensorsGraphModel::LastTemp);
			item->setData (url, SensorsGraphModel::IconURL);
			items << item;

			svg [name] = svgContents.data ();

			QFile file (QDir::homePath () + "/shitfuck");
			file.open (QIODevice::WriteOnly);
			file.write (svgContents.data ());
		}

		++UpdateCounter_;

		GraphProvider_->SetHistory (svg);
		Model_->invisibleRootItem ()->appendRows (items);
	}
}
}
