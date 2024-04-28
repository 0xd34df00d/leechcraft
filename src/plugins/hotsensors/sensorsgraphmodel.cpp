/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sensorsgraphmodel.h"

namespace LC::HotSensors
{
	SensorsGraphModel::SensorsGraphModel (QObject *parent)
	: RoleNamesMixin<QStandardItemModel> (parent)
	{
		setRoleNames ({
				{ LastTemp, "lastTemp" },
				{ SensorName, "sensorName" },
				{ PointsList, "pointsList" },
				{ MaxPointsList, "maxPointsList" },
				{ MaxTemp, "maxTemp" },
				{ CritTemp, "critTemp" },
				{ MaxPointsCount, "maxPointsCount" },
			});
	}
}
