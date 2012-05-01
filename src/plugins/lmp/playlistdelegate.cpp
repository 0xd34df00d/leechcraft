/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "playlistdelegate.h"
#include <QTreeView>
#include <QPainter>
#include <QApplication>
#include <util/util.h>
#include "player.h"
#include "mediainfo.h"

namespace LeechCraft
{
namespace LMP
{
	const int Padding = 2;

	PlaylistDelegate::PlaylistDelegate (QTreeView *view, QObject *parent)
	: QStyledItemDelegate (parent)
	, View_ (view)
	{
	}

	void PlaylistDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& optionOld, const QModelIndex& index) const
	{
		QStyleOptionViewItemV4 option = optionOld;
		const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();

		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

		const bool isAlbum = index.data (Player::Role::IsAlbum).toBool ();
		if (isAlbum)
		{
			style->drawPrimitive (QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
			const int maxIconHeight = option.rect.height () - Padding * 2;
			QPixmap px = index.data (Player::Role::AlbumArt).value<QPixmap> ();
			px = px.scaled (maxIconHeight, maxIconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			painter->drawPixmap (option.rect.left () + Padding, option.rect.top () + Padding, px);

			const QFont& font = option.font;
			QFont bold = font;
			bold.setBold (true);
			QFont italic = font;
			italic.setItalic (true);
			QFont boldItalic = bold;
			boldItalic.setItalic (true);

			int x = option.rect.left () + maxIconHeight + 3 * Padding;
			int y = option.rect.top ();
			painter->save ();

			auto append = [&x, &y, painter] (const QString& text, const QFont& font)
			{
				painter->setFont (font);
				y += QFontMetrics (font).boundingRect (text).height ();
				painter->drawText (x, y, text);
			};
			append (info.Album_, bold);
			append (info.Artist_, boldItalic);
			append (info.Genres_.join (" / "), italic);
			append (QString::number (info.Year_), font);

			const int length = index.data (Player::Role::AlbumLength).toInt ();
			auto lengthStr = Util::MakeTimeFromLong (length);
			if (lengthStr.startsWith ("00:"))
				lengthStr = lengthStr.mid (3);
			painter->drawText (option.rect.adjusted (Padding, Padding, -Padding, -Padding), Qt::AlignRight, lengthStr);

			painter->restore ();

			return;
		}

		const bool isSubAlbum = index.parent ().isValid ();

		QStyleOptionViewItemV4 bgOpt = option;
		painter->save ();
		if (index.data (Player::Role::IsCurrent).toBool ())
		{
			const QColor& highlight = bgOpt.palette.color (QPalette::Highlight);
			QLinearGradient grad (0, 0, 0, bgOpt.rect.height ());
			grad.setColorAt (0, highlight.lighter (100));
			grad.setColorAt (1, highlight.darker (200));
			bgOpt.backgroundBrush = QBrush (grad);

			QFont font = option.font;
			font.setItalic (true);
			painter->setFont (font);
			painter->setPen (bgOpt.palette.color (QPalette::HighlightedText));
		}
		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &bgOpt, painter, option.widget);
		QString lengthText = Util::MakeTimeFromLong (info.Length_);
		if (lengthText.startsWith ("00:"))
			lengthText = lengthText.mid (3);

		const int width = option.fontMetrics.width (lengthText);
		style->drawItemText (painter, option.rect,
				Qt::AlignRight, option.palette, true, lengthText);
		style->drawItemText (painter, option.rect.adjusted (0, 0, -width, 0),
				0, option.palette, true,
				isSubAlbum ?
					QString::fromUtf8 ("%1 — %2").arg (info.TrackNumber_).arg (info.Title_) :
					index.data ().toString ());
		painter->restore ();
	}

	QSize PlaylistDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (!index.data (Player::Role::IsAlbum).toBool ())
			return QStyledItemDelegate::sizeHint (option, index);

		const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();
		const int width = View_->viewport ()->size ().width () - 4;

		const QFont& font = option.font;
		QFont bold = font;
		bold.setBold (true);
		const auto boldFM = QFontMetrics (bold);
		QFont italic = font;
		italic.setItalic (true);
		const auto italicFM = QFontMetrics (italic);
		QFont boldItalic = bold;
		boldItalic.setItalic (true);
		const auto boldItalicFM = QFontMetrics (boldItalic);

		const int height = Padding * 2 +
				boldFM.boundingRect (info.Album_).height () +
				boldItalicFM.boundingRect (info.Artist_).height () +
				italicFM.boundingRect (info.Genres_.join (" / ")).height () +
				option.fontMetrics.boundingRect (QString::number (info.Year_)).height ();

		return QSize (width, std::max (height, 32));
	}
}
}
