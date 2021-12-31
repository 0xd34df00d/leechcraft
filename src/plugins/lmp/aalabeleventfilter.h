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

namespace LC::LMP
{
	class AALabelEventFilter : public QObject
	{
	public:
		using CoverPathGetter_t = std::function<QString ()>;
	private:
		CoverPathGetter_t Getter_;
	public:
		explicit AALabelEventFilter (CoverPathGetter_t, QObject* = nullptr);
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	};
}
