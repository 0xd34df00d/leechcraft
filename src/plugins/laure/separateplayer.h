/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LAURE_SEPARATEPLAYER_H
#define PLUGINS_LAURE_SEPARATEPLAYER_H
#include <QWidget>

namespace LeechCraft
{
namespace Laure
{
	/** @brief Provides a separate video frame widget.
	 *
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class SeparatePlayer : public QWidget
	{
		Q_OBJECT
	public:
		/** @brief Constructs a new SeparatePlayer tab
		 * with the given parent and flags.
		 */
		SeparatePlayer (QWidget *parent = 0, Qt::WindowFlags f = 0);
	protected:
		void closeEvent (QCloseEvent*);
	signals:
		void closed ();
	};
}
}

#endif // PLUGINS_LAURE_SEPARATEPLAYER_H
