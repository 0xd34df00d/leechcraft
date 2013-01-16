/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "artistbrowsertab.h"
#include <QMessageBox>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/ipluginsmanager.h>
#include <util/gui/clearlineeditaddon.h>
#include "similarviewmanager.h"
#include "bioviewmanager.h"
#include "core.h"
#include "previewhandler.h"

namespace LeechCraft
{
namespace LMP
{
	ArtistBrowserTab::ArtistBrowserTab (const TabClassInfo& tc, QObject *plugin)
	: TC_ (tc)
	, Plugin_ (plugin)
	{
		Ui_.setupUi (this);

		BioMgr_ = new BioViewManager (Ui_.View_, this);
		SimilarMgr_ = new SimilarViewManager (Ui_.View_, this);

		Ui_.View_->setSource (QUrl ("qrc:/lmp/resources/qml/ArtistBrowserView.qml"));

		BioMgr_->InitWithSource ();
		SimilarMgr_->InitWithSource ();

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.ArtistNameEdit_);
	}

	TabClassInfo ArtistBrowserTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* ArtistBrowserTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void ArtistBrowserTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ArtistBrowserTab::GetToolBar () const
	{
		return 0;
	}

	void ArtistBrowserTab::Browse (const QString& artist)
	{
		Ui_.ArtistNameEdit_->setText (artist);
		on_ArtistNameEdit__returnPressed ();
	}

	void ArtistBrowserTab::on_ArtistNameEdit__returnPressed ()
	{
		auto provs = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IArtistBioFetcher*> ();
		if (provs.isEmpty ())
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("There aren't any plugins that can fetch biography. Check if "
						"you have installed for example the LastFMScrobble plugin."));
			return;
		}

		auto artist = Ui_.ArtistNameEdit_->text ().trimmed ();

		BioMgr_->Request (provs.first (), artist);
		SimilarMgr_->DefaultRequest (artist);
	}
}
}
