/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glance.h"
#include <QAction>
#include <QTabWidget>
#include <QToolBar>
#include <QMainWindow>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "glanceshower.h"

namespace LC
{
namespace Plugins
{
namespace Glance
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("glance");

		Proxy_ = proxy;
		Core::Instance ().SetProxy (proxy);

		ActionGlance_ = new QAction (GetName (), this);
		ActionGlance_->setToolTip ("Show the quick overview of tabs");
		ActionGlance_->setShortcut (QKeySequence ("Ctrl+Shift+G"));
		ActionGlance_->setShortcutContext (Qt::ApplicationShortcut);
		ActionGlance_->setProperty ("ActionIcon", "view-list-icons");
		ActionGlance_->setProperty ("Action/ID", GetUniqueID () + "_glance");

		connect (ActionGlance_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_ActionGlance__triggered ()));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Glance";
	}

	QString Plugin::GetName () const
	{
		return "Glance";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Quick overview of tabs.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	void Plugin::on_ActionGlance__triggered ()
	{
		Glance_ = new GlanceShower;
		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		Glance_->SetTabWidget (rootWM->GetTabWidget (rootWM->GetPreferredWindowIndex ()));

		connect (Glance_,
				SIGNAL (finished (bool)),
				ActionGlance_,
				SLOT (setEnabled (bool)));

		ActionGlance_->setEnabled (false);
		Glance_->Start ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::QuickLaunch)
			result << ActionGlance_;
		return result;
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		QMap<QString, ActionInfo> result;

		const auto& iconName = ActionGlance_->property ("ActionIcon").toString ();
		result ["ShowList"] = ActionInfo (ActionGlance_->text (),
				ActionGlance_->shortcut (),
				Proxy_->GetIconThemeManager ()->GetIcon (iconName));

		return result;
	}

	void Plugin::SetShortcut (const QString&, const QKeySequences_t& seqs)
	{
		ActionGlance_->setShortcuts (seqs);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_glance, LC::Plugins::Glance::Plugin);
