/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>

namespace LC
{
namespace LMP
{
	class AALabelEventFilter : public QObject
	{
	public:
		typedef std::function<QString ()> CoverPathGetter_t;
	private:
		CoverPathGetter_t Getter_;
	public:
		AALabelEventFilter (CoverPathGetter_t, QObject* = 0);
	protected:
		bool eventFilter (QObject*, QEvent*);
	};
}
}
