/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IAUTHWIDGET_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IAUTHWIDGET_H

#include <QVariant>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class IAuthWidget
	{
	public:
		virtual ~IAuthWidget () {};

		virtual QVariantMap GetIdentifyingData () const = 0;
		virtual void SetIdentifyingData (const QVariantMap&) = 0;
	};
}
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::OnlineBookmarks::IAuthWidget,
		"org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IAuthWidget/1.0")

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IAUTHWIDGET_H
