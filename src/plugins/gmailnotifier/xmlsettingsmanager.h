/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Yury Erik Potapov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_GMAILNOTIFIER__XMLSETTINGSMANAGER_H
#define PLUGINS_GMAILNOTIFIER__XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC
{
namespace GmailNotifier
{
	class XmlSettingsManager: public Util::BaseSettingsManager
	{
		Q_OBJECT

		XmlSettingsManager ();
	public:
		static XmlSettingsManager* Instance ();
	protected:
		virtual QSettings* BeginSettings () const;
		virtual void EndSettings (QSettings*) const;
	};
}
}

#endif
