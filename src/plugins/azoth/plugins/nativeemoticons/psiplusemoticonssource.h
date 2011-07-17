/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_PSIPLUSEMOTICONSSOURCE_H
#define PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_PSIPLUSEMOTICONSSOURCE_H
#include <QObject>
#include <QHash>
#include "baseemoticonssource.h"

namespace LeechCraft
{
namespace Azoth
{
namespace NativeEmoticons
{
	class PsiPlusEmoticonsSource : public BaseEmoticonsSource
	{
		Q_OBJECT

		mutable String2Filename_t IconCache_;
		mutable QString CachedPack_;
	public:
		PsiPlusEmoticonsSource (QObject* = 0);
	private:
		// Hash is chat string → filename.
		String2Filename_t ParseFile (const QString&) const;
	};
}
}
}

#endif
