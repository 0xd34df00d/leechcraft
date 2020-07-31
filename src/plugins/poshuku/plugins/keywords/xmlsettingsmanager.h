/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC
{
namespace Poshuku
{
namespace Keywords
{
    class XmlSettingsManager : public Util::BaseSettingsManager
	{
		XmlSettingsManager ();
	public:
		static XmlSettingsManager* Instance ();
	protected:
		virtual QSettings* BeginSettings () const;
		virtual void EndSettings (QSettings*) const;
	};
}
}
}
