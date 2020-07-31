/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "backendproxy.h"
#include <numeric>
#include <cmath>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/models/rolenamesmixin.h>
#include "backend.h"
#include "cpuloadproxyobj.h"

namespace LC
{
namespace CpuLoad
{
	namespace
	{
		class CpusModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Role
			{
				CpuIdxRole = Qt::UserRole + 1,
				MomentalLoadStr,
				CpuLoadObj
			};

			CpusModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> { parent }
			{
				QHash<int, QByteArray> roleNames;
				roleNames [CpuIdxRole] = "cpuIdx";
				roleNames [CpuLoadObj] = "loadObj";
				roleNames [MomentalLoadStr] = "momentalLoadStr";
				setRoleNames (roleNames);
			}
		};
	}

	BackendProxy::BackendProxy (Backend *backend)
	: QObject { backend }
	, Backend_ { backend }
	, Model_ { new CpusModel { this } }
	{
	}

	QAbstractItemModel* BackendProxy::GetModel () const
	{
		return Model_;
	}

	namespace
	{
		double GetAccumulated (const QMap<LoadPriority, LoadTypeInfo>& map)
		{
			return std::accumulate (map.begin (), map.end (), 0.0,
					[] (double res, const LoadTypeInfo& info) { return info.LoadPercentage_ + res; });
		}

		QString GetAccumulatedStr (const QMap<LoadPriority, LoadTypeInfo>& map)
		{
			const auto percents = static_cast<int> (std::round (GetAccumulated (map) * 100));
			return QString { "%1%" }
					.arg (percents, 2);
		}
	}

	void BackendProxy::update ()
	{
		Backend_->Update ();

		const auto rc = Model_->rowCount ();
		if (rc == Backend_->GetCpuCount ())
		{
			if (rc > 0)
				for (int i = 0; i < rc; ++i)
				{
					const auto& loads = Backend_->GetLoads (i);
					ModelPropObjs_.at (i)->Set (loads);

					const auto item = Model_->item (i);
					item->setData (GetAccumulatedStr (loads), CpusModel::MomentalLoadStr);
				}

			return;
		}

		Model_->removeRows (0, rc);
		qDeleteAll (ModelPropObjs_);
		ModelPropObjs_.clear ();

		if (Backend_->GetCpuCount () <= 0)
			return;

		QList<QStandardItem*> newItems;

		for (int i = 0; i < Backend_->GetCpuCount (); ++i)
		{
			const auto& loads = Backend_->GetLoads (i);

			auto obj = new CpuLoadProxyObj { Backend_->GetLoads (i) };
			ModelPropObjs_ << obj;

			auto modelItem = new QStandardItem;
			modelItem->setData (i, CpusModel::CpuIdxRole);
			modelItem->setData (QVariant::fromValue<QObject*> (obj), CpusModel::CpuLoadObj);
			modelItem->setData (GetAccumulatedStr (loads), CpusModel::MomentalLoadStr);
			newItems << modelItem;
		}

		Model_->invisibleRootItem ()->appendRows (newItems);
	}

	QList<QPointF> BackendProxy::sumPoints (QList<QPointF> list, const QList<QPointF>& other)
	{
		for (auto i = 0; i < list.size (); ++i)
			list [i].ry () += other [i].y ();
		return list;
	}

	QList<QPointF> BackendProxy::enableIf (QList<QPointF> pts, bool flag)
	{
		if (!flag)
			for (auto& p : pts)
				p.ry () = 0;

		return pts;
	}
}
}
