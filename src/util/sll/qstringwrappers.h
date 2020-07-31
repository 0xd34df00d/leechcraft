/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC
{
namespace Util
{
	struct QStringTrimmed
	{
		QString operator() (const QString& s) const
		{
			return s.trimmed ();
		}

		QString operator() (QString&& s) const
		{
			return s.trimmed ();
		}

		QString operator() (QString& s) const
		{
			return std::move (s).trimmed ();
		}
	};

	struct QStringToLower
	{
		QString operator() (const QString& s) const
		{
			return s.toLower ();
		}

		QString operator() (QString&& s) const
		{
			return s.toLower ();
		}

		QString operator() (QString& s) const
		{
			return std::move (s).toLower ();
		}
	};
}
}
