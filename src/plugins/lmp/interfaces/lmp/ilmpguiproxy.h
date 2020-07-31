/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QWidget;
class QString;
class QAction;

namespace LC
{
namespace LMP
{
	class ILMPGuiProxy
	{
	public:
		virtual ~ILMPGuiProxy () {}

		virtual void AddCurrentSongTab (const QString&, QWidget*) const = 0;

		virtual void AddToolbarAction (QAction*) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ILMPGuiProxy, "org.LeechCraft.LMP.ILMPGuiProxy/1.0")
