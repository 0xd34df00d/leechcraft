/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sessionprops.h"
#include <QObject>
#include <QtDebug>

namespace LC::Util
{
	bool IsSessionProp (const QByteArray& name)
	{
		return name.startsWith ("SessionData/");
	}

	DynPropertiesList_t GetSessionProps (const QObject& tab)
	{
		DynPropertiesList_t props;
		for (const auto& name : tab.dynamicPropertyNames ())
			if (IsSessionProp (name))
				props.append ({ name, tab.property (name.constData ()) });
		return props;
	}

	void SetSessionProps (QObject& tab, const DynPropertiesList_t& props)
	{
		for (const auto& [name, value] : props)
			if (IsSessionProp (name))
				tab.setProperty (name.constData (), value);
			else
				qWarning () << "skipping non-session property" << name << "for" << &tab;
	}
}
