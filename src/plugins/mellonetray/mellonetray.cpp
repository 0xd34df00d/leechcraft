/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mellonetray.h"
#include <QIcon>
#include <QtQuick>
#include <util/util.h>
#include "traymodel.h"
#include "iconhandler.h"

namespace LC
{
namespace Mellonetray
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("mellonetray");

		qmlRegisterType<IconHandler> ("Mellonetray", 1, 0, "IconHandler");

		if (TrayModel::Instance ().IsValid ())
		{
			Panel_ = std::make_shared<QuarkComponent> ();
			Panel_->Url_ = Util::GetSysPathUrl (Util::SysPath::QML, "mellonetray", "TrayQuark.qml");
			Panel_->DynamicProps_.append ({ "MT_trayModel", &TrayModel::Instance () });
		}
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Mellonetray";
	}

	void Plugin::Release ()
	{
		TrayModel::Instance ().Release ();
	}

	QString Plugin::GetName () const
	{
		return "Mellonetray";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("System-wide tray area.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		if (Panel_)
			return { Panel_ };
		else
			return {};
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_mellonetray, LC::Mellonetray::Plugin);
