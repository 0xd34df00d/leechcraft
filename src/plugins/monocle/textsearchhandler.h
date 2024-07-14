/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <util/gui/findnotification.h>
#include "interfaces/monocle/idocument.h"

class QGraphicsRectItem;

namespace LC::Monocle
{
	class ISearchableDocument;
	class PageGraphicsItem;

	struct TextSearchHandlerResults
	{
		QString Text_;
		Util::FindNotification::FindFlags FindFlags_;
		QMap<int, QList<PageRelativeRectBase>> Positions_;
	};

	class TextSearchHandler : public QObject
	{
		Q_OBJECT

		ISearchableDocument *SearchableDoc_;
		QVector<PageGraphicsItem*> Pages_;

		QString CurrentSearchString_;

		struct Highlight
		{
			QGraphicsRectItem *Item_ = nullptr;
			PageRelativeRectBase Rect_;
			int PageIdx_ = 0;
		};

		QVector<Highlight> CurrentHighlights_;
		int CurrentRectIndex_ = -1;
	public:
		using QObject::QObject;

		void HandleDoc (IDocument&, const QVector<PageGraphicsItem*>&);

		bool Search (const QString&, Util::FindNotification::FindFlags);
		void SetPreparedResults (const TextSearchHandlerResults&, int selectedItem);
	private:
		bool RequestSearch (const QString&, Util::FindNotification::FindFlags);

		void BuildHighlights (const QMap<int, QList<PageRelativeRectBase>>&);
		void ClearHighlights ();

		void SelectItem (int);
	signals:
		void navigateRequested (const NavigationAction&);

		void gotSearchResults (const TextSearchHandlerResults&);
	};
}
