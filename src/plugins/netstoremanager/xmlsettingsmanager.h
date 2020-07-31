/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_XMLSETTINGSMANAGER_H
#define PLUGINS_NETSTOREMANAGER_XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC
{
namespace NetStoreManager
{
	class XmlSettingsManager : public Util::BaseSettingsManager
	{
		XmlSettingsManager ();
	public:
		static XmlSettingsManager& Instance ();
	protected:
		virtual QSettings* BeginSettings () const;
		virtual void EndSettings (QSettings*) const;
	};
}
}

#endif
