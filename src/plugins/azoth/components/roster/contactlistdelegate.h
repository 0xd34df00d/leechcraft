/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStyledItemDelegate>
#include "core.h"

class QTreeView;

namespace LC
{
namespace Azoth
{
	class ContactListDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

		AvatarsManager& Avatars_;

		QString ActivityIconset_;
		QString MoodIconset_;
		QString SystemIconset_;

		mutable QHash<QString, QIcon> ActivityIconCache_;
		mutable QHash<QString, QIcon> MoodIconCache_;
		mutable QHash<QString, QIcon> SystemIconCache_;

		QTreeView *View_;

		int ContactHeight_ = 24;
		bool ShowAvatars_ = true;
		bool ShowClientIcons_ = true;
		bool ShowStatuses_ = true;
		bool HighlightGroups_ = true;
	public:
		explicit ContactListDelegate (AvatarsManager& avatars, QTreeView* = 0);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
		QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const override;
	private:
		void DrawAccount (QPainter*,
				QStyleOptionViewItem, const QModelIndex&) const;
		void DrawCategory (QPainter*,
				QStyleOptionViewItem, const QModelIndex&) const;
		void DrawContact (QPainter*,
				QStyleOptionViewItem, const QModelIndex&) const;

		QList<QIcon> GetContactIcons (const QModelIndex&, ICLEntry*, const QStringList&) const;
	signals:
		void hookCollectContactIcons (LC::IHookProxy_ptr, QObject*, QList<QIcon>&) const;
	};
}
}
