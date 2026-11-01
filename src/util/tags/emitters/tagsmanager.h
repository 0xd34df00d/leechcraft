/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include "../tagsconfig.h"

namespace LC::Util::Emitters
{
	/** @brief The signals of an `ITagsManager`.
	 *
	 * @sa ITagsManager::GetEmitter()
	 */
	class UTIL_TAGS_API TagsManager : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief Emitted after the name `old` was replaced with `newNames`
		 * on every tag that had it.
		 *
		 * One name renames the tag, several split it, a name that already
		 * exists merges it into that tag, and no names at all remove it.
		 *
		 * @param[out] old The replaced name.
		 * @param[out] newNames The replacing names, trimmed and distinct.
		 */
		void nameReplaced (const QString& old, const QStringList& newNames);
	};
}
