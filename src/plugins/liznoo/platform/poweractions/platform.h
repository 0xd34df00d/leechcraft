/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFuture>

namespace LC
{
namespace Liznoo
{
namespace PowerActions
{
	class Platform : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		enum class State
		{
			Suspend,
			Hibernate
		};

		struct QueryChangeStateResult
		{
			bool CanChangeState_;
			QString Reason_;
		};

		virtual QFuture<bool> IsAvailable () = 0;

		virtual QFuture<QueryChangeStateResult> CanChangeState (State) = 0;
		virtual void ChangeState (State) = 0;
	};
}
}
}
