/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QByteArray>

class QObject;

namespace LC::Auscrie
{
	struct FilterState
	{
		QByteArray PluginId_;
		QByteArray Variant_;
	};

	void SaveFilterState (const FilterState&);
	FilterState RestoreFilterState ();
}
