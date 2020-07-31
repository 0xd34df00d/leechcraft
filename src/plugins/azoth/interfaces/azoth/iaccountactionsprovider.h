/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QAction;

template<typename>
class QList;

namespace LC
{
namespace Azoth
{
	class IAccount;

	class IAccountActionsProvider
	{
	protected:
		virtual ~IAccountActionsProvider () = default;
	public:
		virtual QList<QAction*> CreateActions (IAccount*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IAccountActionsProvider,
		"org.LeechCraft.Azoth.IAccountActionsProvider/1.0")
