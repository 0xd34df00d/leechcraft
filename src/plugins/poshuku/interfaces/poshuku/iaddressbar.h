/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_IADDRESSBAR_H
#define PLUGINS_POSHUKU_IADDRESSBAR_H
#include <QtPlugin>
class QAction;
class QToolButton;

namespace LC
{
namespace Poshuku
{
	class IAddressBar
	{
	public:
		virtual ~IAddressBar () {}

		virtual QObject* GetQObject () = 0;

		virtual int ButtonsCount () const = 0;

		virtual QToolButton* AddAction (QAction *action, bool hideOnEmptyUrl = false) = 0;
		virtual QToolButton* InsertAction (QAction *action, int pos = -1, bool hideOnEmptyUrl = false) = 0;

		virtual void RemoveAction (QAction *action) = 0;

		virtual QToolButton* GetButtonFromAction (QAction *action) const = 0;

		virtual void SetVisible (QAction *action, bool visible) = 0;
	protected:
		virtual void actionTriggered (QAction *action, const QString& text) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IAddressBar,
		"org.Deviant.LeechCraft.Poshuku.IAddressBar/1.0")

#endif
