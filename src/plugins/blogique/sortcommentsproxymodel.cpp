/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2013  Oleg Linkin <MaledicutsDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sortcommentsproxymodel.h"
#include <QtDebug>
#include "commentswidget.h"

namespace LC
{
namespace Blogique
{
	SortCommentsProxyModel::SortCommentsProxyModel (CommentsWidget *widget, QObject *parent)
	: QSortFilterProxyModel (parent)
	, CommentsWidget_ (widget)
	{
		setDynamicSortFilter (true);
		setSortCaseSensitivity (Qt::CaseInsensitive);
	}

	bool SortCommentsProxyModel::lessThan (const QModelIndex& left, const QModelIndex& right) const
	{
		const auto& leftRecentCommment = CommentsWidget_->GetRecentCommentFromIndex (left);
		const auto& rightRecentCommment = CommentsWidget_->GetRecentCommentFromIndex (right);
		return leftRecentCommment.CommentDateTime_ < rightRecentCommment.CommentDateTime_;
	}
}
}
