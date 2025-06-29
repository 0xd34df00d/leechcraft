/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QSize>
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include "geometry.h"

namespace LC::Util
{
	namespace
	{
		class AADisplayEventFilter : public QObject
		{
			QWidget * const Display_;
		public:
			explicit AADisplayEventFilter (QWidget *display)
			: QObject (display)
			, Display_ (display)
			{
			}
		protected:
			bool eventFilter (QObject*, QEvent *event) override
			{
				bool shouldClose = false;
				switch (event->type ())
				{
				case QEvent::KeyRelease:
					shouldClose = static_cast<QKeyEvent*> (event)->key () == Qt::Key_Escape;
					break;
				case QEvent::MouseButtonRelease:
					shouldClose = true;
					break;
				default:
					break;
				}

				if (!shouldClose)
					return false;

				QTimer::singleShot (0,
						Display_,
						&QWidget::close);
				return true;
			}
		};
	}

	QLabel* ShowPixmapLabel (const QPixmap& srcPx, const QPoint& centerPos)
	{
		const auto scaleFactor = 0.9;
		const auto& availGeom = AvailableGeometry (centerPos);
		const auto& availSize = availGeom.size () * scaleFactor;

		auto px = srcPx;
		if (px.size ().width () > availSize.width () ||
			px.size ().height () > availSize.height ())
			px = px.scaled (availSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		auto topLeftPos = centerPos - QPoint { px.size ().width (), px.size ().height () } /2;
		if (!availGeom.contains (topLeftPos))
		{
			topLeftPos.setX (std::max (topLeftPos.x (), availGeom.left ()));
			topLeftPos.setY (std::max (topLeftPos.y (), availGeom.top ()));
		}

		const auto label = new QLabel;
		label->setWindowFlags (Qt::Tool);
		label->setAttribute (Qt::WA_DeleteOnClose);
		label->setFixedSize (px.size ());
		label->setPixmap (px);
		label->show ();
		label->activateWindow ();
		label->installEventFilter (new AADisplayEventFilter (label));
		label->move (topLeftPos);
		return label;
	}

	QColor TintColors (const QColor& c1, const QColor& c2, double alpha)
	{
		QColor color;
		color.setRedF (alpha * c1.redF () + (1 - alpha) * c2.redF ());
		color.setGreenF (alpha * c1.greenF () + (1 - alpha) * c2.greenF ());
		color.setBlueF (alpha * c1.blueF () + (1 - alpha) * c2.blueF ());
		return color;
	}

	QString ElideProgressBarText (const QString& text, const QStyleOptionViewItem& option)
	{
		return option.fontMetrics.elidedText (text, Qt::ElideRight, option.rect.width ());
	}

	void TintPalette (QWidget *widget, const QColor& color, double alpha, const QList<QPalette::ColorRole>& roles)
	{
		auto palette = widget->palette ();
		for (auto role : roles)
			palette.setColor (role, TintColors (palette.color (role), color, alpha));
		widget->setPalette (palette);
	}

	QString FormatName (const QString& name)
	{
		return "<em>" + name + "</em>";
	}

	QPixmap DrawOverlayText (QPixmap px,
			const QString& text, QFont font, const QPen& pen, const QBrush& brush)
	{
		const auto& iconSize = px.size () / px.devicePixelRatio ();

		const auto fontHeight = iconSize.height () * 0.45;
		const auto minFontHeight = 6.0;
		font.setPixelSize (static_cast<int> (std::max (minFontHeight, fontHeight)));

		const QFontMetrics fm (font);
		const auto width = fm.horizontalAdvance (text) + 2. * iconSize.width () / 10.;
		const auto height = fm.height () + 2. * iconSize.height () / 10.;
		const bool tooSmall = width > iconSize.width ();

		const QRectF textRect (iconSize.width () - width, iconSize.height () - height, width, height);

		QPainter p (&px);
		p.setBrush (brush);
		p.setFont (font);
		p.setPen (pen);
		p.setRenderHint (QPainter::Antialiasing);
		p.setRenderHint (QPainter::TextAntialiasing);
		p.drawRoundedRect (textRect, 4, 4);
		p.drawText (textRect,
				Qt::AlignCenter,
				tooSmall ? QStringLiteral ("#") : text);
		p.end ();

		return px;
	}

	// https://bugreports.qt.io/browse/QTBUG-53550
	QIcon FixupTrayIcon (const QIcon& icon)
	{
		if (!icon.availableSizes ().isEmpty ())
			return icon;

		constexpr auto pxSize = 256;
		return QIcon { icon.pixmap (pxSize, pxSize) };
	}

	namespace
	{
		QString MakeFileDialogFilterImpl (auto&& entries)
		{
			const auto toString = [] (const auto& e) { return e.Description_ + " (*." + e.Extension_ + ")"; };
			return MapAs<QList> (entries, toString).join (";;"_ql);
		}
	}

	QString MakeFileDialogFilter (std::initializer_list<FileDialogFilterEntry> entries)
	{
		return MakeFileDialogFilterImpl (entries);
	}

	QString MakeFileDialogFilter (const QList<FileDialogFilterEntry>& entries)
	{
		return MakeFileDialogFilterImpl (entries);
	}
}
