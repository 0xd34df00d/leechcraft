/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QUrl>
#include <QDateTime>
#include <QPixmap>
#include <QMetaType>
#include <QMap>

namespace LC
{
namespace Poshuku
{
	/** Enumeration describing the part of menu that's being constructed
	 * inside QWebView's subclass' contextMenuEvent.
	 */
	enum WebViewCtxMenuStage
	{
		/// Just the beginning of menu construction.
		WVSStart,
		/// Stage related to clicking on a hyperlink finished.
		WVSAfterLink,
		/// Stage related to clicking on an image finished.
		WVSAfterImage,
		/// Stage related to clicking with having some selected text
		/// finished.
		WVSAfterSelectedText,
		/// The standard set of actions was embedded. This stage is just
		/// before executing the menu.
		WVSAfterFinish
	};

	struct HistoryItem
	{
		QString Title_;
		QDateTime DateTime_;
		QString URL_;
	};

	typedef QList<HistoryItem> history_items_t;

	struct ContextMenuInfo
	{
		bool IsContentEditable_;

		QString SelectedPageText_;

		QUrl LinkUrl_;
		QString LinkText_;

		QUrl ImageUrl_;
		QPixmap ImagePixmap_;
	};

	struct ElementData
	{
		QUrl PageURL_;
		QString FormID_;
		QString Name_;
		QString Type_;
		QString Value_;
	};

	inline bool operator== (const ElementData& left, const ElementData& right)
	{
		return left.PageURL_ == right.PageURL_ &&
			   left.FormID_ == right.FormID_ &&
			   left.Name_ == right.Name_ &&
			   left.Type_ == right.Type_ &&
			   left.Value_ == right.Value_;
	}

	inline bool operator< (const ElementData& left, const ElementData& right)
	{
		if (left.PageURL_ != right.PageURL_)
			return left.PageURL_ < right.PageURL_;

		if (left.FormID_ != right.FormID_)
			return left.FormID_ < right.FormID_;

		if (left.Name_ != right.Name_)
			return left.Name_ < right.Name_;

		if (left.Type_ != right.Type_)
			return left.Type_ < right.Type_;

		return left.Value_ < right.Value_;
	}

	using ElementsData_t = QList<ElementData>;

	/** Holds information about all the forms on a page.
	 *
	 * The key of the map is the name of the `input' element, whereas
	 * value is the ElementData structure with the information about
	 * that element.
	 */
	using PageFormsData_t = QMap<QString, ElementsData_t>;
}
}

Q_DECLARE_METATYPE (LC::Poshuku::ElementData)
Q_DECLARE_METATYPE (LC::Poshuku::ElementsData_t)
Q_DECLARE_METATYPE (LC::Poshuku::PageFormsData_t)
