/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "guiconfig.h"

namespace LC::Util
{
	/** @brief A horizontal widget embedding into the parent layout of
	 * the passed \em parent widget.
	 *
	 * This is a base class for FindNotification, for example.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API PageNotification : public QWidget
	{
	public:
		/** @brief Creates the widget embedding into the parent layout of
		 * the \em parent widget.
		 *
		 * The \em parent widget is expected to be contained in a
		 * QVBoxLayout.
		 *
		 * @param[in] parent The widget whose parent layout should be used.
		 */
		explicit PageNotification (QWidget *parent);
	};
}
