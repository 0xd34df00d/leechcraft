/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QMetaType>

namespace LC::Fenet
{
	struct Param
	{
		QString Name_;
		QString Desc_;

		double Default_;

		double Min_;
		double Max_;
	};

	struct Flag
	{
		QString Name_;
		QString Desc_;
	};

	struct CompInfo
	{
		QList<Param> Params_;
		QList<Flag> Flags_;

		QString Name_;
		QString Comment_;

		QStringList ExecNames_;
	};

	typedef QList<CompInfo> CompInfos_t;
}

Q_DECLARE_METATYPE (LC::Fenet::Param)
Q_DECLARE_METATYPE (LC::Fenet::Flag)
Q_DECLARE_METATYPE (LC::Fenet::CompInfo)
