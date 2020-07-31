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

		bool ShowAvatars_;
		bool ShowClientIcons_;
		bool ShowStatuses_;
		bool HighlightGroups_;
		int ContactHeight_;
		QString ActivityIconset_;
		QString MoodIconset_;
		QString SystemIconset_;
		mutable QHash<QString, QIcon> ActivityIconCache_;
		mutable QHash<QString, QIcon> MoodIconCache_;
		mutable QHash<QString, QIcon> SystemIconCache_;
		QTreeView *View_;
	public:
		ContactListDelegate (QTreeView* = 0);

		virtual void paint (QPainter*,
				const QStyleOptionViewItem&, const QModelIndex&) const;
		virtual QSize sizeHint (const QStyleOptionViewItem&,
				const QModelIndex&) const;
	private:
		void DrawAccount (QPainter*,
				QStyleOptionViewItem, const QModelIndex&) const;
		void DrawCategory (QPainter*,
				QStyleOptionViewItem, const QModelIndex&) const;
		void DrawContact (QPainter*,
				QStyleOptionViewItem, const QModelIndex&) const;

		QList<QIcon> GetContactIcons (const QModelIndex&, ICLEntry*, const QStringList&) const;

		void LoadSystemIcon (const QString&, QList<QIcon>&) const;
	private slots:
		void handleShowAvatarsChanged ();
		void handleShowClientIconsChanged ();
		void handleActivityIconsetChanged ();
		void handleMoodIconsetChanged ();
		void handleSystemIconsetChanged ();
		void handleShowStatusesChanged ();
		void handleHighlightGroupsChanged ();
		void handleContactHeightChanged ();
	signals:
		void hookCollectContactIcons (LC::IHookProxy_ptr, QObject*, QList<QIcon>&) const;
	};
}
}
