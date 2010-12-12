/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "contactlistdelegate.h"
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include "interfaces/iclentry.h"
#include "core.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
	const int CContactShift = 20;
	const int CPadding = 2;

	ContactListDelegate::ContactListDelegate (QObject* parent)
	: QStyledItemDelegate (parent)
	{
	}

	void ContactListDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& sopt, const QModelIndex& index) const
	{
		QStyleOptionViewItemV4 option = sopt;
		Core::CLEntryType type = index.data (Core::CLREntryType).value<Core::CLEntryType> ();

		painter->save ();

		switch (type)
		{
		case Core::CLETAccount:
			option.font.setBold (true);
		case Core::CLETCategory:
			option.rect.setLeft (0);
			QStyledItemDelegate::paint (painter, option, index);
			break;
		case Core::CLETContact:
			DrawContact (painter, option, index);
			break;
		}

		painter->restore ();
	}

	QSize ContactListDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize size = QStyledItemDelegate::sizeHint (option, index);
		return size;
	}

	void ContactListDelegate::DrawContact (QPainter *painter,
			QStyleOptionViewItemV4 option, const QModelIndex& index) const
	{
		QObject *entryObj = index.data (Core::CLREntryObject).value<QObject*> ();
		Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (entryObj);

		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

		option.rect.setLeft (CContactShift);

		const QIcon& stateIcon = index.data (Qt::DecorationRole).value<QIcon> ();
		QString name = index.data (Qt::DisplayRole).value<QString> ();
		const QString& status = entry->GetStatus ().StatusString_;
		const QImage& avatarImg = entry->GetAvatar ();
		if (!status.isEmpty ())
			name += " (" + status + ")";

		const bool selected = option.state & QStyle::State_Selected;
		const QColor fgColor = selected ?
				option.palette.color (QPalette::HighlightedText) :
				option.palette.color (QPalette::Text);
		const QRect& r = option.rect;
		const int sHeight = r.height ();
		const int iconSize = sHeight - 2 * CPadding;
		const int textShift = r.left () + 2 * CPadding + iconSize;
		const int textWidth = r.width () - textShift -
				(avatarImg.isNull () ?
					CPadding :
					iconSize + 2 * CPadding);

		QPixmap pixmap (r.size ());
		pixmap.fill (Qt::transparent);
		QPainter p (&pixmap);
		p.translate (-r.topLeft ());

		if (selected ||
				(option.state & QStyle::State_MouseOver))
			style->drawPrimitive (QStyle::PE_PanelItemViewItem,
					&option, &p, option.widget);

		p.setPen (fgColor);
		p.setFont (option.font);
		p.drawText (textShift, r.top () + CPadding,
				textWidth, r.height () - 2 * CPadding,
				Qt::AlignVCenter | Qt::AlignLeft,
				option.fontMetrics.elidedText (name, Qt::ElideRight, textWidth));

		p.drawPixmap (r.topLeft () + QPoint (CPadding, CPadding),
				stateIcon.pixmap (iconSize, iconSize));

		if (!avatarImg.isNull ())
			p.drawPixmap (r.topLeft () + QPoint (textShift + textWidth + CPadding, CPadding),
					QPixmap::fromImage (avatarImg.scaled (iconSize, iconSize)));

		painter->drawPixmap (option.rect, pixmap);
	}
}
}
}
