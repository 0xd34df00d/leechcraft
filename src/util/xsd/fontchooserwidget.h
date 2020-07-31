/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_fontchooserwidget.h"

namespace LC
{
namespace Util
{
	/** @brief Provides a widget for choosing a font.
	 *
	 * This widget allows the user to choose a font in the XML settings
	 * dialog.
	 */
	class FontChooserWidget : public QWidget
	{
		Q_OBJECT

		Ui::FontChooserWidget Ui_;
	public:
		/** @brief Constructs the widget with the given parent.
		 *
		 * @param[in] parent The parent widget.
		 */
		FontChooserWidget (QWidget *parent = nullptr);

		/** @brief Returns the currently selected font.
		 *
		 * @return The currently chosen font.
		 *
		 * @sa SetFont()
		 */
		QFont GetFont () const;

		/** @brief Sets the currently font to \em font.
		 *
		 * @param[in] font The font to set the currently chosen one to.
		 *
		 * @sa GetFont()
		 */
		void SetFont (const QFont& font);
	signals:
		/** @brief Emitted when another font has been chosen.
		 *
		 * @param[out] font The new chosen font.
		 */
		void fontChanged (QFont font);
	};
}
}
