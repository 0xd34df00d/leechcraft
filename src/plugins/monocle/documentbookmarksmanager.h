/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/monocle/idocument.h"

class QStandardItemModel;
class QAbstractItemModel;

namespace LC::Monocle
{
	class DocumentTab;

	class DocumentBookmarksManager : public QObject
	{
		Q_OBJECT

		DocumentTab * const Tab_;
		QStandardItemModel * const Model_;

		IDocument_ptr Doc_;
	public:
		DocumentBookmarksManager (DocumentTab*, QObject* = nullptr);

		QAbstractItemModel* GetModel () const;

		void HandleDoc (IDocument_ptr);
		bool HasDoc () const;

		void AddBookmark ();
		void RemoveBookmark (QModelIndex);

		void Navigate (const QModelIndex&);
	private:
		void ReloadBookmarks ();
	signals:
		void docAvailable (bool);
	};
}
