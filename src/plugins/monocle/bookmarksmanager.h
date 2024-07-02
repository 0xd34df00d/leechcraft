/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QtXml/QDomDocument>
#include "interfaces/monocle/idocument.h"

namespace LC
{
namespace Monocle
{
	class Bookmark;

	class BookmarksManager : public QObject
	{
		Q_OBJECT

		QDomDocument BookmarksDOM_;
	public:
		BookmarksManager (QObject* = nullptr);

		void AddBookmark (const IDocument&, const Bookmark&);
		void RemoveBookmark (const IDocument&, const Bookmark&);

		QVector<Bookmark> GetBookmarks (const IDocument&) const;
	private:
		QDomElement GetDocElem (const QString&) const;
		QDomElement GetDocElem (const QString&);

		void Load ();
		bool LoadSaved ();
		void Save () const;
	signals:
		void documentBookmarksChanged (const IDocument*);
	};
}
}
