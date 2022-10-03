/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QObject>

class QMenu;

namespace LC::Azoth
{
	class IAccount;
}

namespace LC::Azoth::Actions
{
	struct AccountActions
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::AccountActions)
	};

	void PopulateMenu (QMenu*, IAccount*);
}
