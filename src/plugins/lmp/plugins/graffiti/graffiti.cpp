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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/lmp/mediainfo.h>
#include "graffititab.h"
#include "progressmanager.h"

namespace LC
{
namespace LMP
{
namespace Graffiti
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("lmp_graffiti");

		CoreProxy_ = proxy;

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
		return "LMP Graffiti";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to manipulate audio files tags.");
	}

	QIcon Plugin::GetIcon () const
	{
		return CoreProxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.General";
		return result;
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
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
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
		auto tab = new GraffitiTab (CoreProxy_, LMPProxy_, TaggerTC_, this);
		emit addNewTab (TaggerTC_.VisibleName_, tab);
		emit raiseTab (tab);

		connect (tab,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));

		connect (tab,
				SIGNAL (tagsFetchProgress (int, int, QObject*)),
				ProgressMgr_,
				SLOT (handleTagsFetch (int, int, QObject*)));
		connect (tab,
				SIGNAL (cueSplitStarted (CueSplitter*)),
				ProgressMgr_,
				SLOT (handleCueSplitter (CueSplitter*)));
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
				SLOT (handleOpenTabFromContextMenu ()));
		action->setProperty ("ActionIcon", "mail-tagged");
		action->setProperty ("LMP/Graffiti/Filepath", mediaInfo.LocalPath_);
	}

	void Plugin::hookCollectionContextMenuRequested (IHookProxy_ptr proxy,
			QMenu *menu, const MediaInfo& info)
	{
		hookPlaylistContextMenuRequested (proxy, menu, info);
	}

	void Plugin::handleOpenTabFromContextMenu ()
	{
		const auto& path = sender ()->property ("LMP/Graffiti/Filepath").toString ();

		const auto tab = MakeTab ();

		const QFileInfo info { path };
		tab->SetPath (info.dir ().path (), info.fileName ());
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_graffiti, LC::LMP::Graffiti::Plugin);
