/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class XmlSettingsManager : public LC::Util::BaseSettingsManager
	{
		XmlSettingsManager ();
	public:
		static XmlSettingsManager& Instance ();
	protected:
		QSettings* BeginSettings () const override;
		void EndSettings (QSettings*) const override;
	};
}
}
}
