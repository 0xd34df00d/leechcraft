/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "secman.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "contentsdisplaydialog.h"
#include "persistentstorage.h"

namespace LC
{
namespace SecMan
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("secman");

#ifdef SECMAN_EXPOSE_CONTENTSDISPLAY
		auto displayContentsAction = new QAction (tr ("Display storages' contents"), this);
		connect (displayContentsAction,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDisplayContents ()));
		MenuActions_ ["tools"] << displayContentsAction;
#endif
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SecMan";
	}

	QString Plugin::GetName () const
	{
		return "SecMan";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Secure data storage for other LeechCraft modules.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return {};
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		return MenuActions_;
	}

	IPersistentStorage_ptr Plugin::RequestStorage ()
	{
		return std::make_shared<PersistentStorage> ();
	}

	void Plugin::handleDisplayContents ()
	{
		auto dia = new ContentsDisplayDialog;
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_secman, LC::SecMan::Plugin);
