/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "tagsconfig.h"

namespace LC::Util
{
	class CategorySelector;

	/** @brief A `CategorySelector` in a window of its own, kept within the screen.
	 *
	 * The variants differ only in the buttons: `Buttons::Close` dismisses the
	 * window, `Buttons::AcceptReject` gives OK and Cancel for `exec()`. The window
	 * is titled "Tags selector" until `setWindowTitle()` says otherwise.
	 */
	class UTIL_TAGS_API CategorySelectorWindow : public QDialog
	{
		Q_OBJECT

		CategorySelector& Selector_;
	public:
		enum class Buttons
		{
			Close,
			AcceptReject
		};

		explicit CategorySelectorWindow (Buttons, QWidget *parent = nullptr);

		CategorySelector& Selector ();
	protected:
		void moveEvent (QMoveEvent*) override;
	};
}
