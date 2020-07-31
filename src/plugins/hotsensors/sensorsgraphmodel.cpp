/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sensorsgraphmodel.h"

namespace LC
{
namespace HotSensors
{
	SensorsGraphModel::SensorsGraphModel (QObject *parent)
	: RoleNamesMixin<QStandardItemModel> (parent)
	{
		QHash<int, QByteArray> roleNames;
		roleNames [LastTemp] = "lastTemp";
		roleNames [SensorName] = "sensorName";
		roleNames [PointsList] = "pointsList";
		roleNames [MaxPointsList] = "maxPointsList";
		roleNames [MaxTemp] = "maxTemp";
		roleNames [CritTemp] = "critTemp";
		roleNames [MaxPointsCount] = "maxPointsCount";
		setRoleNames (roleNames);
	}
}
}
