/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistdelegate.h"
#include <QTreeView>
#include <QPainter>
#include <QApplication>
#include <util/util.h>
#include <interfaces/structures.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>
#include "player.h"
#include "mediainfo.h"
#include "util.h"

Q_DECLARE_METATYPE (QList<LC::Entity>)

namespace LC
{
namespace LMP
{
	const int Padding = 2;

	PlaylistDelegate::PlaylistDelegate (QTreeView *view, QObject *parent, const ICoreProxy_ptr& proxy)
	: QStyledItemDelegate (parent)
	, View_ (view)
	, Proxy_ (proxy)
	{
	}

	void PlaylistDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& optionOld, const QModelIndex& index) const
	{
		const bool isAlbum = index.data (Player::Role::IsAlbum).toBool ();

		auto option = optionOld;
		auto& pal = option.palette;
		if (!(option.features & QStyleOptionViewItem::Alternate))
		{
			QLinearGradient grad (0, 0, option.rect.width (), 0);
			grad.setColorAt (0, pal.color (QPalette::Window).darker (105));
			grad.setColorAt (0.5, pal.color (QPalette::Window).darker (120));
			grad.setColorAt (1, pal.color (QPalette::Window).darker (105));
			option.backgroundBrush = QBrush (grad);
		}

		if (isAlbum)
			PaintAlbum (painter, option, index);
		else
			PaintTrack (painter, option, index);
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

	void PlaylistDelegate::PaintOneShot (const QVariant& oneShotPosVar,
			QStyleOptionViewItem& option, QPainter *painter, QStyle *style, bool drawSelected) const
	{
		if (!oneShotPosVar.isValid ())
			return;

		const auto& text = QString::number (oneShotPosVar.toInt () + 1);
		const auto& textWidth = option.fontMetrics.horizontalAdvance (text);

		auto oneShotRect = option.rect;
		oneShotRect.setWidth (std::max (textWidth + 2 * Padding, oneShotRect.height ()));

		painter->save ();
		painter->setRenderHint (QPainter::Antialiasing);
		painter->setRenderHint (QPainter::TextAntialiasing);
		painter->setBrush (option.palette.brush (drawSelected ?
					QPalette::Highlight :
					QPalette::Button));
		painter->setPen (option.palette.color (QPalette::ButtonText));
		painter->drawEllipse (oneShotRect);
		painter->restore ();

		style->drawItemText (painter,
				oneShotRect,
				Qt::AlignCenter,
				option.palette,
				true,
				text);

		option.rect.adjust (oneShotRect.width () + Padding, 0, 0, 0);
	}

	void PlaylistDelegate::PaintRules (const QVariant& rulesVar,
			QStyleOptionViewItem& option, QPainter *painter, QStyle *style) const
	{
		if (!rulesVar.isValid ())
			return;

		const auto flagWidth = option.fontMetrics.horizontalAdvance (GetRuleSymbol ({}).first);
		const auto rectWidth = std::max (flagWidth + 2 * Padding, option.rect.height ());

		for (const auto& rule : rulesVar.value<QList<Entity>> ())
		{
			painter->save ();

			auto ruleRect = option.rect;
			ruleRect.setWidth (rectWidth);

			const auto& ruleSymb = GetRuleSymbol (rule);
			const auto& color = ruleSymb.second;
			if (color.isValid ())
				painter->setPen (color);

			style->drawItemText (painter,
					ruleRect,
					Qt::AlignCenter,
					option.palette,
					true,
					ruleSymb.first);

			painter->restore ();

			option.rect.adjust (ruleRect.width () + Padding, 0, 0, 0);
		}
	}

	void PlaylistDelegate::PaintTrack (QPainter *painter,
			QStyleOptionViewItem option, const QModelIndex& index) const
	{
		const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();

		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

		const bool isSubAlbum = index.parent ().isValid ();

		auto bgOpt = option;

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
		}

		const bool drawSelected = index.data (Player::Role::IsCurrent).toBool () ||
				option.state & QStyle::State_Selected;
		if (drawSelected)
			painter->setPen (bgOpt.palette.color (QPalette::HighlightedText));

		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &bgOpt, painter, option.widget);

		PaintOneShot (index.data (Player::Role::OneShotPos), option, painter, style, drawSelected);

		if (index.data (Player::Role::IsStop).toBool ())
		{
			const auto& icon = Proxy_->GetIconThemeManager ()->GetIcon ("media-playback-stop");
			const auto& px = icon.pixmap (option.rect.size ());
			style->drawItemPixmap (painter, option.rect, Qt::AlignLeft | Qt::AlignVCenter, px);

			option.rect.adjust (px.width () + Padding, 0, 0, 0);
		}

		PaintRules (index.data (Player::Role::MatchingRules), option, painter, style);

		QString lengthText = Util::MakeTimeFromLong (info.Length_);
		if (lengthText.startsWith ("00:"))
			lengthText = lengthText.mid (3);
		const int width = option.fontMetrics.horizontalAdvance (lengthText);
		style->drawItemText (painter, option.rect,
				Qt::AlignRight, option.palette, true, lengthText);

		const auto& itemTextRect = option.rect.adjusted (0, 0, -width, 0);
		QString itemStr;
		if (!isSubAlbum)
			itemStr = index.data ().toString ();
		else
		{
			if (info.TrackNumber_ > 0 && !info.Title_.isEmpty ())
				itemStr = QString::fromUtf8 ("%1 — %2").arg (info.TrackNumber_).arg (info.Title_);
			else if (!info.Title_.isEmpty ())
				itemStr = info.Title_;
			else
				itemStr = QFileInfo (info.LocalPath_).fileName ();
		}
		itemStr = option.fontMetrics.elidedText (itemStr, Qt::ElideRight, itemTextRect.width ());

		style->drawItemText (painter, itemTextRect, 0, option.palette, true, itemStr);
		painter->restore ();
	}

	void PlaylistDelegate::PaintAlbum (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const auto& info = index.data (Player::Role::Info).value<MediaInfo> ();

		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

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

		if (option.state & QStyle::State_Selected)
			painter->setPen (option.palette.color (QPalette::HighlightedText));

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
	}
}
}
