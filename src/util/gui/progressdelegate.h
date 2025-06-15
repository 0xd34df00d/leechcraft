/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStyledItemDelegate>
#include "guiconfig.h"

namespace LC::Util
{
	class UTIL_GUI_API ProgressDelegate : public QStyledItemDelegate
	{
	public:
		struct Progress
		{
			int Minimum_ = 0;
			int Maximum_;
			int Progress_;
			QString Text_;
		};

		using ProgressGetter_t = std::function<Progress (QModelIndex)>;
	private:
		ProgressGetter_t ProgressGetter_;
	public:
		explicit ProgressDelegate (ProgressGetter_t&& progress, QObject *parent = nullptr);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
	};
}
