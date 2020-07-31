/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <util/qml/widthiconprovider.h>
#include <util/xdg/xdgfwd.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Launchy
{
	class ItemImageProvider : public Util::WidthIconProvider
	{
		const ICoreProxy_ptr Proxy_;
		QHash<QString, QIcon> PermID2Icon_;
	public:
		explicit ItemImageProvider (const ICoreProxy_ptr&);

		void AddItem (Util::XDG::Item_ptr);

		QIcon GetIcon (const QStringList&) override;
	};
}
}
