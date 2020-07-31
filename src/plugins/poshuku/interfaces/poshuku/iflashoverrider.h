/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_INTERFACES_IFLASHOVERRIDER_H
#define PLUGINS_POSHUKU_INTERFACES_IFLASHOVERRIDER_H
#include <QtPlugin>

class QUrl;

namespace LC
{
namespace Poshuku
{
	class IFlashOverrider
	{
	public:
		virtual ~IFlashOverrider () {}

		virtual bool WouldOverrideFlash (const QUrl&) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IFlashOverrider,
		"org.Deviant.LeechCraft.Poshuku.IFlashOverrider/1.0")

#endif
