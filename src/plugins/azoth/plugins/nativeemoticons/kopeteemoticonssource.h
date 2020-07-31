/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_KOPETEEMOTICONSSOURCE_H
#define PLUGINS_AZOTH_PLUGINS_NATIVEEMOTICONS_KOPETEEMOTICONSSOURCE_H
#include <QObject>
#include <QHash>
#include "baseemoticonssource.h"

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	class KopeteEmoticonsSource : public BaseEmoticonsSource
	{
		Q_OBJECT

		mutable String2Filename_t IconCache_;
		mutable QString CachedPack_;
	public:
		KopeteEmoticonsSource (QObject* = 0);
	private:
		// Hash is chat string → filename.
		String2Filename_t ParseFile (const QString&) const;
	};
}
}
}

#endif
