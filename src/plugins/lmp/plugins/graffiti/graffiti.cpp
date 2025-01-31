/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "graffiti.h"
#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include <util/util.h>
#include <util/lmp/mediainfo.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "cuesplitter.h"
#include "literals.h"
#include "graffititab.h"
#include "progressmanager.h"

namespace LC::LMP::Graffiti
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator (QStringLiteral ("lmp_graffiti"));

		ProgressMgr_ = new ProgressManager ();

		TaggerTC_ =
		{
			GetUniqueID () + "_Tagger",
			"LMP Graffiti",
			GetInfo (),
			GetIcon (),
			0,
			TFOpenableByRequest
		};
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.Graffiti";
	}

	QString Plugin::GetName () const
	{
		return Lits::LMPGraffiti;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to manipulate audio files tags.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.LMP.General" };
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TaggerTC_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (TaggerTC_.TabClass_ == tabClass)
			MakeTab ();
		else
			qWarning () << "unknown tab class"
					<< tabClass;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return ProgressMgr_->GetModel ();
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}

	GraffitiTab* Plugin::MakeTab ()
	{
		auto tab = new GraffitiTab (LMPProxy_, TaggerTC_, this);
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (TaggerTC_.VisibleName_, tab);

		connect (tab,
				&GraffitiTab::tagsFetchProgress,
				ProgressMgr_,
				&ProgressManager::HandleTagsFetch);
		connect (tab,
				&GraffitiTab::cueSplitStarted,
				ProgressMgr_,
				&ProgressManager::HandleCueSplitter);

		return tab;
	}

	void Plugin::hookPlaylistContextMenuRequested (LC::IHookProxy_ptr,
			QMenu *menu, const MediaInfo& mediaInfo)
	{
		if (mediaInfo.LocalPath_.isEmpty ())
			return;

		const QFileInfo info { mediaInfo.LocalPath_ };
		if (!info.exists ())
			return;

		menu->addSeparator ();

		const auto action = menu->addAction (tr ("Edit tags..."),
				this,
				[this, path = mediaInfo.LocalPath_]
				{
					const QFileInfo info { path };
					MakeTab ()->SetPath (info.dir ().path (), info.fileName ());
				});
		action->setProperty ("ActionIcon", "mail-tagged");
	}

	void Plugin::hookCollectionContextMenuRequested (IHookProxy_ptr proxy,
			QMenu *menu, const MediaInfo& info)
	{
		hookPlaylistContextMenuRequested (proxy, menu, info);
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_graffiti, LC::LMP::Graffiti::Plugin);
