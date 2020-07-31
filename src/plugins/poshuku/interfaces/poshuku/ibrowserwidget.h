/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QWebView;
class QLineEdit;
class QMenu;
class QString;

namespace LC
{
namespace Poshuku
{
	class IWebView;

	class IBrowserWidget
	{
	public:
		virtual ~IBrowserWidget () {}

		virtual QLineEdit* GetURLEdit () const = 0;

		virtual IWebView* GetWebView () const = 0;

		virtual void InsertFindAction (QMenu *menu, const QString& text) = 0;

		virtual void AddStandardActions (QMenu *menu) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IBrowserWidget,
		"org.Deviant.LeechCraft.Poshuku.IBrowserWidget/1.0")
