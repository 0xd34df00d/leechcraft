/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <util/utilconfig.h>

class QLineEdit;
class QToolButton;

namespace LeechCraft
{
namespace Util
{
	/** @brief Provides a "clear text" action for line edits.
	 *
	 * Using this class is as simple as this:
	 * \code
	 * QLineEdit *edit = ...; // or some QLineEdit-derived class
	 * new ClearLineEditAddon (proxy, edit); // proxy is the one passed to IInfo::Init()
	 * \endcode
	 *
	 * The constructor takes a pointer to the proxy object that is passed
	 * to IInfo::Init() method of the plugin instance object and the
	 * pointer to the line edit where the addon should be installed.
	 *
	 * The line edit takes the ownership of the addon, so there is no
	 * need to keep track of it or explicitly delete it.
	 *
	 * @sa IInfo::Init()
	 */
	class UTIL_API ClearLineEditAddon : public QObject
	{
		Q_OBJECT

		QToolButton *Button_;
		QLineEdit *Edit_;
	public:
		/** @brief Creates the addon and installs it on the given edit.
		 *
		 * @param[in] proxy The proxy passed to IInfo::Init() of the
		 * plugin.
		 * @param[in] edit The line edit to install this addon into. The
		 * edit takes ownership of the addon.
		 */
		ClearLineEditAddon (ICoreProxy_ptr proxy, QLineEdit *edit);
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		void UpdatePos ();
	private slots:
		void updateButton (const QString&);
	};
}
}
