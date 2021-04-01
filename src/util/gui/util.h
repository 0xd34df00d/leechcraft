/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPoint>
#include <QPalette>
#include <QMimeData>
#include "guiconfig.h"

class QSize;
class QRect;
class QPixmap;
class QLabel;
class QColor;
class QWidget;
class QStyleOptionViewItem;

namespace LC::Util
{
	/** @brief Shows a pixmap at the given pos.
	 *
	 * This function shows a dialog with the given pixmap at the given
	 * position. If the pixmap is too big, it is scaled down. A QLabel
	 * created with window decorations is used as the dialog. The created
	 * label is returned from the function, so one could also set the
	 * window title and further customize the label.
	 *
	 * This function is useful to display full version of album art in a
	 * media player or a user avatar in an IM application.
	 *
	 * @param[in] pixmap The pixmap to show.
	 * @param[in] pos The position where the dialog should be shown.
	 * @return The created dialog.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API QLabel* ShowPixmapLabel (const QPixmap& pixmap, const QPoint& pos = QPoint ());

	/** @brief Mixes two colors with the given weights.
	 *
	 * Mixes two colors \em c1 and \em c2 with weights \em alpha and
	 * (1 - \em alpha) respectively.
	 *
	 * @param[in] c1 The first color to mix.
	 * @param[in] c2 The second color to mix.
	 * @param[in] alpha The weight of the first color (and
	 * <code>1 - weight</code> for the second color).
	 * @return The mixed color.
	 *
	 * @sa TintPalette()
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API QColor TintColors (const QColor& c1, const QColor& c2, double alpha = 0.5);

	UTIL_GUI_API QString ElideProgressBarText (const QString& text, const QStyleOptionViewItem& option);

	/** @brief Mixes some of the widget's palette roles with the given
	 * color.
	 *
	 * This function queries the palette of the \em widget and mixes the
	 * colors for the given \em roles in it with the given \em color and
	 * the given \em alpha weight. After that it sets the resulting
	 * palette back for the \em widget.
	 *
	 * @param[in] widget The widget whose palette should be changed.
	 * @param[in] color The color to tint the palette's colors with.
	 * @param[in] alpha The weight of the palette's colors in the
	 * resulting mix.
	 * @param[in] roles The roles of the palette that should be mixed with
	 * the given color.
	 *
	 * @sa TintColors()
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API void TintPalette (QWidget *widget,
			const QColor& color,
			double alpha = 0.5,
			const QList<QPalette::ColorRole>& roles = { QPalette::ColorRole::Text, QPalette::ColorRole::WindowText });

	/** @brief HTML-formats the \em name to let the user know it is not a part of
	 * the fixed dialog text.
	 *
	 * This function is useful to format an account name, a link title and
	 * similar entities in contexts like a dialog, a label or a tooltip. Using
	 * this function ensures this formatting is done in an uniform style across
	 * the whole GUI.
	 *
	 * @param[in] name The source string.
	 * @return HTML-formatted \em name.
	 */
	UTIL_GUI_API QString FormatName (const QString& name);

	UTIL_GUI_API QPixmap DrawOverlayText (QPixmap px, const QString& text, QFont font, const QPen& pen, const QBrush& brush);

	template<typename T>
	void Save2MimeData (QMimeData *mimeData, const QString& name, const T& t)
	{
		QByteArray infosData;
		QDataStream ostr { &infosData, QIODevice::WriteOnly };
		ostr << t;

		mimeData->setData (name, infosData);
	}
}

namespace LC
{
	// TODO make this consteval when clang will stop crashing on this being consteval
	inline constexpr QColor operator"" _color (const char *str, std::size_t size)
	{
		if (size != 7)
			throw std::runtime_error { "invalid color size" };

		constexpr auto digit = [] (char digit)
		{
			if (digit >= '0' && digit <= '9')
				return digit - '0';
			if (digit >= 'a' && digit <= 'f')
				return digit - 'a' + 0xa;
			if (digit >= 'A' && digit <= 'F')
				return digit - 'A' + 0xa;

			throw std::runtime_error { "unable to parse" };
		};

		constexpr auto group = [digit] (const char *str)
		{
			return digit (str [0]) * 16 + digit (str [1]);
		};

		return QColor { group (str + 1), group (str + 3), group (str + 5) };
	}
}
