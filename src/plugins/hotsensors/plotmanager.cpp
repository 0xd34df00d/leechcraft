/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "plotmanager.h"
#include <QStandardItemModel>
#include <QPainter>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <util/sll/qtutil.h>
#include "contextwrapper.h"
#include "sensorsgraphmodel.h"
#include "historymanager.h"

Q_DECLARE_METATYPE (QList<QPointF>)

namespace LC
{
namespace HotSensors
{
	PlotManager::PlotManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new SensorsGraphModel (this))
	{
	}

	QAbstractItemModel* PlotManager::GetModel () const
	{
		return Model_;
	}

	std::unique_ptr<QObject> PlotManager::CreateContextWrapper ()
	{
		return std::make_unique<ContextWrapper> (GetModel (), Proxy_ );
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

		for (const auto& pair : Util::Stlize (history))
		{
			const auto& name = pair.first;

			double max = 0, crit = 0;
			QList<QPointF> points;
			QList<QPointF> maxPoints;
			for (const auto& item : pair.second)
			{
				const auto index = static_cast<qreal> (points.size ());
				points.append ({ index, item.Value_ });
				maxPoints.append ({ index, item.Max_ });
				max = std::max (max, item.Max_);
				crit = std::max (crit, item.Crit_);
			}

			for (int i = maxPoints.size (); i < HistoryManager::GetMaxHistorySize (); ++i)
				maxPoints.append ({ static_cast<qreal> (i), max });

			const bool isKnownSensor = existing.contains (name);
			auto item = isKnownSensor ? existing.take (name) : new QStandardItem;

			const auto lastTemp = pair.second.empty () ?
					0 :
					static_cast<int> (pair.second.back ().Value_);
			item->setData (QString::fromUtf8 ("%1°C").arg (lastTemp), SensorsGraphModel::LastTemp);
			item->setData (name, SensorsGraphModel::SensorName);
			item->setData (QVariant::fromValue (points), SensorsGraphModel::PointsList);
			item->setData (QVariant::fromValue (maxPoints), SensorsGraphModel::MaxPointsList);
			item->setData (max, SensorsGraphModel::MaxTemp);
			item->setData (crit, SensorsGraphModel::CritTemp);
			item->setData (HistoryManager::GetMaxHistorySize (),
					SensorsGraphModel::MaxPointsCount);
			if (!isKnownSensor)
				items << item;
		}

		for (auto item : existing)
			Model_->removeRow (item->row ());

		if (!items.isEmpty ())
			Model_->invisibleRootItem ()->appendRows (items);
	}
}
}
