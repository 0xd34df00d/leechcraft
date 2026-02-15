/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "modeldelegate.h"
#include <QApplication>
#include <QStyle>
#include <util/gui/util.h>
#include <interfaces/ijobholder.h>
#include <interfaces/structures.h>
#include "summarytagsfilter.h"

namespace LC
{
namespace Summary
{
	namespace
	{
		bool DrawProgress (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index)
		{
			auto done = index.data (+JobHolderProcessRole::Done).value<qlonglong> ();
			auto total = index.data (+JobHolderProcessRole::Total).value<qlonglong> ();
			if (done < 0 || total <= 0)
				return false;

			while (done > 1000 && total > 1000)
			{
				done /= 10;
				total /= 10;
			}

			QStyleOptionProgressBar pbo;
			pbo.rect = option.rect;
			pbo.minimum = 0;
			pbo.maximum = total;
			pbo.progress = done;
			pbo.state = option.state | QStyle::StateFlag::State_Horizontal;
			pbo.text = Util::ElideProgressBarText (index.data ().toString (), option);
			pbo.textVisible = true;
			QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
			return true;
		}
	}

	void ModelDelegate::paint (QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.column () == SummaryTagsFilter::Progress)
		{
			const auto rowInfo = index.data (+JobHolderRole::RowInfo).value<RowInfo> ();
			if (const auto isProcess = std::holds_alternative<ProcessInfo> (rowInfo.Specific_);
				isProcess && DrawProgress (painter, option, index))
				return;
		}

		QStyledItemDelegate::paint (painter, option, index);
	}
}
}
