/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "packagesdelegate.h"
#include <QPainter>
#include <QApplication>
#include "packagesmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			const int PackagesDelegate::CPadding = 5;
			const int PackagesDelegate::CIconSize = 32;
			const int PackagesDelegate::CTitleSizeDelta = 2;
			const int PackagesDelegate::CNumLines = 3;

			PackagesDelegate::PackagesDelegate (QObject *parent)
			: QStyledItemDelegate (parent)
			{
			}

			void PackagesDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				QStyleOptionViewItemV4 opt (option);
				QStyle *style = opt.widget ?
						opt.widget->style () :
						QApplication::style ();
				QFontMetrics fontMetrics = opt.widget ?
						opt.widget->fontMetrics () :
						QApplication::fontMetrics ();

				const QRect& r = option.rect;
				bool ltr = (painter->layoutDirection () == Qt::LeftToRight);

				QString title = index.data (Qt::DisplayRole).toString ();
				QString shortDescr = index.data (PackagesModel::PMRShortDescription).toString ();
				QStringList tags = index.data (PackagesModel::PMRTags).toStringList ();

				QString attribute;
				QColor fgColor = option.state.testFlag (QStyle::State_Selected) ?
						option.palette.color (QPalette::HighlightedText) :
						option.palette.color (QPalette::Text);

				QIcon icon = index.data (Qt::DecorationRole).value<QIcon> ();

				QStyleOptionViewItem titleOption (option);
				titleOption.font.setBold (true);
				titleOption.font.setPointSize (titleOption.font.pointSize () + CTitleSizeDelta);

				QPixmap pixmap (option.rect.size ());
				pixmap.fill (Qt::transparent);
				QPainter p (&pixmap);
				p.translate (-option.rect.topLeft ());

				const int height = ItemHeightForOption (option);
				int textShift = 2 * CPadding + CIconSize;
				int textWidth = r.width () - textShift;
				int singleHeight = height / CNumLines;

				p.setPen (fgColor);
				p.setFont (titleOption.font);
				p.drawText (r.left () + (ltr ? textShift : 0), r.top (),
						textWidth, singleHeight,
						Qt::AlignBottom | Qt::AlignLeft, title);

				p.setFont (option.font);
				shortDescr = fontMetrics.elidedText (shortDescr,
						option.textElideMode, textWidth);
				p.drawText (r.left () + (ltr ? textShift : 0), r.top () + singleHeight,
						textWidth, singleHeight,
						Qt::AlignTop | Qt::AlignLeft, shortDescr);

				QFont tagsFont = option.font;
				tagsFont.setItalic (true);
				p.setFont (tagsFont);
				p.drawText (r.left () + (ltr ? textShift : 0), r.top () + 2 * singleHeight,
						textWidth, singleHeight,
						Qt::AlignBottom | Qt::AlignRight, tags.join ("; "));

				p.end ();

				painter->drawPixmap (option.rect, pixmap);

				icon.paint (painter,
						ltr ?
								r.left () + CPadding :
								r.left () + r.width () - CPadding - CIconSize,
						r.top () + CPadding,
						CIconSize, CIconSize,
						Qt::AlignCenter, QIcon::Normal);
			}

			QSize PackagesDelegate::sizeHint (const QStyleOptionViewItem& option,
					const QModelIndex& index) const
			{
				QSize result = index.data (Qt::SizeHintRole).toSize ();
				result.rheight () = ItemHeightForOption (option);
				return result;
			}

			int PackagesDelegate::ItemHeightForOption (const QStyleOptionViewItem& option) const
			{
				QFont boldFont = option.font;

				boldFont.setBold (true);
				boldFont.setPointSize (boldFont.pointSize () + CTitleSizeDelta);

				int textHeight = QFontInfo (boldFont).pixelSize () + QFontInfo (option.font).pixelSize () * (CNumLines - 1);
				return std::max (textHeight, CIconSize) + 2 * CPadding;
			}
		}
	}
}
