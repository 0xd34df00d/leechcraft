/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDir>
#include "common.h"

namespace LC::Monocle
{
	class DocStateManager : public QObject
	{
		QDir DocDir_;
	public:
		struct State
		{
			int CurrentPage_;
			LayoutMode Lay_;
			ScaleMode ScaleMode_;
		};

		explicit DocStateManager (QObject* = nullptr);

		void SetState (const QString&, const State&);
		State GetState (const QString&) const;
	};
}
