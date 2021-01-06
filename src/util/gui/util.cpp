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
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QtDebug>
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

	QLabel* ShowPixmapLabel (const QPixmap& srcPx, const QPoint& pos)
	{
		const auto scaleFactor = 0.9;
		const auto& availGeom = AvailableGeometry (pos).size () * scaleFactor;

		auto px = srcPx;
		if (px.size ().width () > availGeom.width () ||
			px.size ().height () > availGeom.height ())
			px = px.scaled (availGeom, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		const auto label = new QLabel;
		label->setWindowFlags (Qt::Tool);
		label->setAttribute (Qt::WA_DeleteOnClose);
		label->setFixedSize (px.size ());
		label->setPixmap (px);
		label->show ();
		label->activateWindow ();
		label->installEventFilter (new AADisplayEventFilter (label));
		label->move (pos);
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
		font.setPixelSize (std::max (6., fontHeight));

		const QFontMetrics fm (font);
		const auto width = fm.horizontalAdvance (text) + 2. * iconSize.width () / 10.;
		const auto height = fm.height () + 2. * iconSize.height () / 10.;
		const bool tooSmall = width > iconSize.width ();

		const QRect textRect (iconSize.width () - width, iconSize.height () - height, width, height);

		QPainter p (&px);
		p.setBrush (brush);
		p.setFont (font);
		p.setPen (pen);
		p.setRenderHint (QPainter::Antialiasing);
		p.setRenderHint (QPainter::TextAntialiasing);
		p.drawRoundedRect (textRect, 4, 4);
		p.drawText (textRect,
				Qt::AlignCenter,
				tooSmall ? "#" : text);
		p.end ();

		return px;
	}
}
