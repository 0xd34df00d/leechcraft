/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lmpproxy.h"
#include <QToolBar>
#include "util.h"
#include "localcollection.h"
#include "localfileresolver.h"
#include "previewhandler.h"
#include "playertab.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	QString LMPUtilProxy::FindAlbumArt (const QString& nearPath, bool includeCollection) const
	{
		return FindAlbumArtPath (nearPath, !includeCollection);
	}

	QList<QFileInfo> LMPUtilProxy::RecIterateInfo (const QString& path,
			bool followSymLinks, std::atomic<bool> *stopGuard) const
	{
		return LMP::RecIterateInfo (path, followSymLinks, stopGuard);
	}

	void LMPGuiProxy::SetPlayerTab (PlayerTab *tab)
	{
		PlayerTab_ = tab;
	}

	void LMPGuiProxy::AddCurrentSongTab (const QString& title, QWidget *widget) const
	{
		PlayerTab_->AddNPTab (title, widget);
	}

	void LMPGuiProxy::AddToolbarAction (QAction *action) const
	{
		PlayerTab_->GetToolBar ()->addAction (action);
	}

	LMPProxy::LMPProxy (ILocalCollection *lc, ITagResolver *tr, PreviewHandler *ph)
	: LocalCollection_ { lc }
	, TagResolver_ { tr }
	, PreviewHandler_ { ph }
	{
	}

	ILocalCollection* LMPProxy::GetLocalCollection () const
	{
		return LocalCollection_;
	}

	ITagResolver* LMPProxy::GetTagResolver () const
	{
		return TagResolver_;
	}

	const ILMPUtilProxy* LMPProxy::GetUtilProxy () const
	{
		return &UtilProxy_;
	}

	const ILMPGuiProxy* LMPProxy::GetGuiProxy () const
	{
		return &GuiProxy_;
	}

	LMPGuiProxy* LMPProxy::GetGuiProxy ()
	{
		return &GuiProxy_;
	}

	void LMPProxy::PreviewRelease (const QString& artist,
			const QString& release, const QList<QPair<QString, int>>& tracks) const
	{
		PreviewHandler_->previewAlbum (artist, release, tracks);
	}
}
}
