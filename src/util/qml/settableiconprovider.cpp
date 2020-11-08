/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settableiconprovider.h"

namespace LC::Util
{
	void SettableIconProvider::SetIcon (const QStringList& path, const QIcon& icon)
	{
		Icons_ [path] = icon;
	}

	void SettableIconProvider::ClearIcon (const QStringList& path)
	{
		Icons_.remove (path);
	}

	QIcon SettableIconProvider::GetIcon (const QStringList& path)
	{
		return Icons_ [path];
	}
}
