/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "progressdelegate.h"
#include <QApplication>
#include "util.h"

namespace LC::Util
{
	ProgressDelegate::ProgressDelegate (ProgressGetter_t&& progress, QObject *parent)
	: QStyledItemDelegate { parent }
	, ProgressGetter_ { std::move (progress) }
	{
	}

	void ProgressDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QStyleOptionProgressBar progressBarOption;
		progressBarOption.state = option.state | QStyle::StateFlag::State_Horizontal;
		progressBarOption.direction = option.direction;
		progressBarOption.rect = option.rect;
		progressBarOption.fontMetrics = option.fontMetrics;
		progressBarOption.palette = option.palette;
		progressBarOption.textAlignment = Qt::AlignCenter;
		progressBarOption.textVisible = true;

		const auto& progress = ProgressGetter_ (index);
		progressBarOption.minimum = progress.Minimum_;
		progressBarOption.maximum = progress.Maximum_;
		progressBarOption.progress = progress.Progress_;
		progressBarOption.text = ElideProgressBarText (progress.Text_, option);

		QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &progressBarOption, painter);
	}
}
