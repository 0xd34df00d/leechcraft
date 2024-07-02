/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarksmanager.h"
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
#include <util/sys/paths.h>
#include "bookmark.h"

namespace LC
{
namespace Monocle
{
	BookmarksManager::BookmarksManager (QObject *parent)
	: QObject (parent)
	{
		Load ();
	}

	namespace
	{
		QString GetDocID (const IDocument& doc)
		{
			const auto& info = doc.GetDocumentInfo ();
			if (!info.Title_.trimmed ().isEmpty ())
				return info.Title_;

			return QFileInfo (doc.GetDocURL ().path ()).fileName ();
		}
	}

	void BookmarksManager::AddBookmark (const IDocument& doc, const Bookmark& bm)
	{
		auto fileElem = GetDocElem (GetDocID (doc));

		auto elem = BookmarksDOM_.createElement ("bm");
		bm.ToXML (elem, BookmarksDOM_);
		fileElem.appendChild (elem);

		Save ();

		emit documentBookmarksChanged (&doc);
	}

	void BookmarksManager::RemoveBookmark (const IDocument& doc, const Bookmark& bm)
	{
		auto fileElem = GetDocElem (GetDocID (doc));

		auto bmElem = fileElem.firstChildElement ("bm");
		while (!bmElem.isNull ())
		{
			auto next = bmElem.nextSiblingElement ("bm");
			if (Bookmark::FromXML (bmElem) == bm)
				fileElem.removeChild (bmElem);
			bmElem = next;
		}

		Save ();

		emit documentBookmarksChanged (&doc);
	}

	QVector<Bookmark> BookmarksManager::GetBookmarks (const IDocument& doc) const
	{
		QVector<Bookmark> result;

		auto fileElem = GetDocElem (GetDocID (doc));
		auto bmElem = fileElem.firstChildElement ("bm");
		while (!bmElem.isNull ())
		{
			result << Bookmark::FromXML (bmElem);
			bmElem = bmElem.nextSiblingElement ("bm");
		}

		return result;
	}

	QDomElement BookmarksManager::GetDocElem (const QString& id)
	{
		auto fileElem = BookmarksDOM_.documentElement ().firstChildElement ("doc");
		while (!fileElem.isNull ())
		{
			if (fileElem.attribute ("id") == id)
				break;

			fileElem = fileElem.nextSiblingElement ("doc");
		}

		if (fileElem.isNull ())
		{
			fileElem = BookmarksDOM_.createElement ("doc");
			fileElem.setAttribute ("id", id);
			BookmarksDOM_.documentElement ().appendChild (fileElem);
		}
		return fileElem;
	}

	QDomElement BookmarksManager::GetDocElem (const QString& id) const
	{
		auto fileElem = BookmarksDOM_.documentElement ().firstChildElement ("doc");
		while (!fileElem.isNull ())
		{
			if (fileElem.attribute ("id") == id)
				return fileElem;

			fileElem = fileElem.nextSiblingElement ("doc");
		}
		return QDomElement ();
	}

	void BookmarksManager::Load ()
	{
		if (LoadSaved ())
			return;

		auto docElem = BookmarksDOM_.createElement ("bookmarks");
		docElem.setTagName ("bookmarks");
		docElem.setAttribute ("version", "1");
		BookmarksDOM_.appendChild (docElem);
	}

	bool BookmarksManager::LoadSaved ()
	{
		auto dir = Util::CreateIfNotExists ("monocle");
		if (!dir.exists ("bookmarks.xml"))
			return false;

		QFile file (dir.absoluteFilePath ("bookmarks.xml"));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return false;
		}

		if (!BookmarksDOM_.setContent (&file))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing file"
					<< file.fileName ();
			return false;
		}

		return true;
	}

	void BookmarksManager::Save () const
	{
		auto dir = Util::CreateIfNotExists ("monocle");
		QFile file (dir.absoluteFilePath ("bookmarks.xml"));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		file.write (BookmarksDOM_.toByteArray (2));
	}
}
}
