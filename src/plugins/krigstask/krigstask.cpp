/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "krigstask.h"
#include <QIcon>
#include <util/util.h>
#include "windowsmodel.h"
#include "taskbarproxy.h"

namespace LC
{
namespace Krigstask
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("krigstask");

		auto model = new WindowsModel;

		Panel_.reset (new QuarkComponent);
		const auto& path = Util::GetSysPath (Util::SysPath::QML, "krigstask", "TaskbarQuark.qml");
		Panel_->Url_ = QUrl::fromLocalFile (path);
		Panel_->DynamicProps_.append ({ "KT_appsModel", model });
		Panel_->DynamicProps_.append ({ "KT_taskbarProxy", new TaskbarProxy (proxy) });
		Panel_->ImageProviders_.append ({ "TaskbarIcons", model->GetImageProvider ()});
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Krigstask";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Krigstask";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Application switcher.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { Panel_ };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_krigstask, LC::Krigstask::Plugin);
