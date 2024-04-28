/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/circular_buffer.hpp>
#include <QString>
#include <QList>
#include <QMap>

namespace LC::HotSensors
{
	struct Reading
	{
		QString Name_;
		double Value_;
		double Max_;
		double Crit_;
	};

	using Readings_t = boost::circular_buffer<Reading>;

	using ReadingsHistory_t = QMap<QString, Readings_t>;
}
