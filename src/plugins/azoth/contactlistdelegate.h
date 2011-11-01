/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_CONTACTLISTDELEGATE_H
#define PLUGINS_AZOTH_CONTACTLISTDELEGATE_H
#include <QStyledItemDelegate>
#include "core.h"

class QTreeView;

namespace LeechCraft
{
namespace Azoth
{
	class ContactListDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

		bool ShowAvatars_;
		bool ShowClientIcons_;
		bool ShowStatuses_;
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
				QStyleOptionViewItemV4, const QModelIndex&) const;
		void DrawCategory (QPainter*,
				QStyleOptionViewItemV4, const QModelIndex&) const;
		void DrawContact (QPainter*,
				QStyleOptionViewItemV4, const QModelIndex&) const;
		void LoadSystemIcon (const QString&, QList<QIcon>&) const;
	private slots:
		void handleShowAvatarsChanged ();
		void handleShowClientIconsChanged ();
		void handleActivityIconsetChanged ();
		void handleMoodIconsetChanged ();
		void handleSystemIconsetChanged ();
		void handleShowStatusesChanged ();
		void handleContactHeightChanged ();
	};
}
}

#endif
