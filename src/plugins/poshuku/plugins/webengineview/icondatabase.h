/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

class QUrl;
class QIcon;

namespace LC::Poshuku::WebEngineView
{
	class IconDatabaseOnDisk;

	template<typename V>
	class StringPathTrie;

	class IconDatabase : public QObject
	{
		std::shared_ptr<IconDatabaseOnDisk> DB_;
		std::shared_ptr<StringPathTrie<QUrl>> Trie_;
	public:
		explicit IconDatabase (QObject* = nullptr);

		void UpdateIcon (const QUrl& pageUrl, const QIcon& icon, const QUrl& iconUrl);
		QIcon GetIcon (const QUrl& pageUrl);
	private:
		void MarkUrl (const QUrl&, const QUrl&);
	};
}
