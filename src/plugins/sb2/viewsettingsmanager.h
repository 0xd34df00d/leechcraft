/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Util
{
	class BaseSettingsManager;
}
namespace SB2
{
	class ViewManager;

	class ViewSettingsManager : public QObject
	{
		ViewManager * const ViewMgr_;
		const std::shared_ptr<Util::BaseSettingsManager> XSM_;
		const Util::XmlSettingsDialog_ptr XSD_;
	public:
		explicit ViewSettingsManager (ViewManager*);

		Util::XmlSettingsDialog* GetXSD () const;
		Util::BaseSettingsManager* GetXSM () const;
	};
}
}
