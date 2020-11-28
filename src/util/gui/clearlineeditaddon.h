/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include "guiconfig.h"

class QLineEdit;
class QToolButton;
class QShortcut;

namespace LC::Util
{
	class LineEditButtonManager;

	/** @brief Provides a "clear text" action for line edits.
	 *
	 * Using this class is as simple as this:
	 * \code
		QLineEdit *edit = ...; // or some QLineEdit-derived class
		new ClearLineEditAddon { proxy, edit }; // proxy is the one passed to IInfo::Init()
	   \endcode
	 *
	 * The constructor takes a pointer to the proxy object that is passed
	 * to IInfo::Init() method of the plugin instance object and the
	 * pointer to the line edit where the addon should be installed.
	 *
	 * The line edit takes the ownership of the addon, so there is no
	 * need to keep track of it or explicitly delete it.
	 *
	 * @sa IInfo::Init()
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API ClearLineEditAddon : public QObject
	{
		Q_OBJECT

		QToolButton * const Button_;
		QShortcut * const EscShortcut_;
	public:
		/** @brief Creates the addon and installs it on the given edit.
		 *
		 * Please note that if you want to use this addon with other
		 * buttons on the line edit you may consider using another
		 * constructor overload, taking an additional
		 * LineEditButtonManager parameter.
		 *
		 * @param[in] proxy The proxy passed to IInfo::Init() of the
		 * plugin.
		 * @param[in] edit The line edit to install this addon into. The
		 * edit takes ownership of the addon.
		 */
		ClearLineEditAddon (const ICoreProxy_ptr& proxy, QLineEdit *edit);

		/** @brief Creates the addon and installs it on the given edit.
		 *
		 * This constructors allows the line edit addon to be used easily
		 * with other buttons installed on the line \em edit using the
		 * passed button \em manager.
		 *
		 * @param[in] proxy The proxy passed to IInfo::Init() of the
		 * plugin.
		 * @param[in] edit The line edit to install this addon into. The
		 * edit takes ownership of the addon.
		 * @param[in] manager The line edit button manager to use instead
		 * of the default internal one.
		 */
		ClearLineEditAddon (const ICoreProxy_ptr& proxy, QLineEdit *edit, LineEditButtonManager *manager);

		/** @brief Toggles whether Esc button clears the line edit.
		 *
		 * By default, Esc clears the line edit.
		 *
		 * @param[in] clears Whether pressing Esc should clear the line
		 * edit.
		 */
		void SetEscClearsEdit (bool clears);
	};
}
