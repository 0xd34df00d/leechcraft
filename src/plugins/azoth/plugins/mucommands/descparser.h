/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMap>
#include <QString>
#include <QCoreApplication>

namespace LC
{
namespace Azoth
{
struct StaticCommand;

namespace MuCommands
{
	class DescParser
	{
		Q_DECLARE_TR_FUNCTIONS (Descriptions)

		struct Desc
		{
			QString Description_;
			QString Help_;
		};

		QMap<QString, Desc> Cmd2Desc_;
	public:
		DescParser ();

		void operator() (StaticCommand&) const;
	};
}
}
}
