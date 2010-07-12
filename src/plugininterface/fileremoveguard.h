/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGININTERFACE_FILEREMOVEGUARD_H
#define PLUGININTERFACE_FILEREMOVEGUARD_H
#include <QFile>
#include "piconfig.h"

namespace LeechCraft
{
	namespace Util
	{
		/** Makes sure that the file represented by this object is
		 * removed when the corresponding instance of this class is
		 * destructed. Useful to automatically remove temporary files,
		 * for example.
		 */
		class PLUGININTERFACE_API FileRemoveGuard : public QFile
		{
		public:
			FileRemoveGuard (const QString&);

			/** Tries to close and remove the file represented by this
			 * object.
			 */
			virtual ~FileRemoveGuard ();
		};
	}
}

#endif
