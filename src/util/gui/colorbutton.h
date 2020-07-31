/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPushButton>
#include <QColor>
#include "guiconfig.h"

namespace LC
{
namespace Util
{
	/** @brief A button for choosing a color.
	 *
	 * This class provides a button that can be used to choose a color.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API ColorButton : public QPushButton
	{
		Q_OBJECT

		QColor Color_;
	public:
		/** @brief Constructs the button with the given \em parent.
		 *
		 * @param[in] parent The parent widget for the button.
		 */
		ColorButton (QWidget *parent = 0);

		/** @brief Returns the current color represented by this button.
		 *
		 * The default value for the color is black.
		 *
		 * @return The currently set color.
		 *
		 * @sa SetColor()
		 */
		QColor GetColor () const;

		/** @brief Sets the color represented by this button.
		 *
		 * Sets the \em color of this button and emits the colorChanged()
		 * signal if the color has been changed.
		 *
		 * @param[in] color The new color to be represented by this
		 * button.
		 *
		 * @sa SetColor(), colorChanged()
		 */
		void SetColor (const QColor& color);
	private slots:
		void handleSelector ();
	signals:
		/** @brief Emitted when the color is changed.
		 *
		 * @param[out] color The new color represented by this button.
		 *
		 * @sa SetColor()
		 */
		void colorChanged (const QColor& color);
	};
}
}
