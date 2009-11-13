/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef DEBUGMESSAGEHANDLER_H
#define DEBUGMESSAGEHANDLER_H
#include <QtGlobal>
#include <QMutex>

namespace DebugHandler
{
	/** Simple debug message handler. Writes the message of type type
	 * to the logs. The logs are contained in ~/.leechcraft and have
	 * following names for different message types:
	 * - QtDebugMsg -> debug.log
	 * - QtWarningMsg -> warning.log
	 * - QtCriticalMsg -> critical.log
	 * - QtFatalMsg -> fatal.log
	 *
	 * @param[in] type The type of the message.
	 * @param[in] message The message to print.
	 *
	 * @sa backtraced
	 */
	void simple (QtMsgType type, const char *message);

	/** Debug message handler which prints backtraces for all messages
	 * except QtDebugMsg ones. This is the only difference from the
	 * simple() debug message handler. Refer to simple() documentation
	 * for more information.
	 *
	 * @param[in] type The type of the message.
	 * @param[in] message The message to print.
	 *
	 * @sa simple
	 */
	void backtraced (QtMsgType type, const char *message);
};

#endif

