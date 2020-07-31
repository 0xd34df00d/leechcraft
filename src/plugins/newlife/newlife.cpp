/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newlife.h"
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QTranslator>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "common/imimportpage.h"
#include "importwizard.h"

namespace LC
{
namespace NewLife
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("newlife");

		Proxy_ = proxy;

		ImporterAction_ = new QAction (tr ("Import settings..."), this);
		ImporterAction_->setProperty ("ActionIcon", "document-import");
		connect (ImporterAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (runWizard ()));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NewLife";
	}

	QString Plugin::GetName () const
	{
		return "New Life";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("The settings importer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		if (place == ActionsEmbedPlace::ToolsMenu)
			result << ImporterAction_;

		return result;
	}

	void Plugin::runWizard ()
	{
		(new ImportWizard (Proxy_, this))->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_newlife, LC::NewLife::Plugin);
