/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_LEGACYENTITYTIMEEXT_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_LEGACYENTITYTIMEEXT_H
#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class LegacyEntityTimeExt : public QXmppClientExtension
	{
	public:
		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);
	};
}
}
}

#endif
