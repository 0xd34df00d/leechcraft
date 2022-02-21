/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "artistbrowsertab.h"
#include <QMessageBox>
#include <QQuickWidget>
#include <interfaces/iinfo.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/qml/standardnamfactory.h>
#include <util/sll/qtutil.h>
#include <util/sll/udls.h>
#include <util/sys/paths.h>
#include "bioviewmanager.h"
#include "literals.h"
#include "similarviewmanager.h"

namespace LC::LMP
{
	ArtistBrowserTab::ArtistBrowserTab (const QString& artist, const DynPropertiesList_t& props)
	: View_ { new QQuickWidget }
	, BioMgr_ { new BioViewManager { View_, this } }
	, SimilarMgr_ { new SimilarViewManager { View_, this } }
	{
		Ui_.setupUi (this);

		View_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
		View_->setResizeMode (QQuickWidget::SizeRootObjectToView);
		layout ()->addWidget (View_);

		new Util::StandardNAMFactory (Lits::LmpSlashQml,
				[] { return 50_mib; },
				View_->engine ());
		View_->setSource (Util::GetSysPathUrl (Util::SysPath::QML,
				Lits::LmpQmlSubdir,
				QStringLiteral ("ArtistBrowserView.qml")));

		connect (Ui_.ArtistNameEdit_,
				&QLineEdit::returnPressed,
				[this] { DoQueries (Ui_.ArtistNameEdit_->text ().trimmed ()); });

		for (const auto& pair : props)
			setProperty (pair.first, pair.second);

		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (GetStaticTabClass ().VisibleName_, this);

		if (!artist.isEmpty ())
		{
			Ui_.ArtistNameEdit_->setText (artist);
			DoQueries (artist);
		}
	}

	const TabClassInfo& ArtistBrowserTab::GetStaticTabClass ()
	{
		static const TabClassInfo tc
		{
			"org.LeechCraft.LMP_artistBrowser",
			tr ("Artist browser"),
			tr ("Allows one to browse information about different artists."),
			QIcon ("lcicons:/lmp/resources/images/lmp_artist_browser.svg"),
			35,
			TFSuggestOpening | TFOpenableByRequest
		};
		return tc;
	}

	TabClassInfo ArtistBrowserTab::GetTabClassInfo () const
	{
		return GetStaticTabClass ();
	}

	QObject* ArtistBrowserTab::ParentMultiTabs ()
	{
		return GetPluginInstance ();
	}

	void ArtistBrowserTab::Remove ()
	{
		emit removeTab ();
	}

	QToolBar* ArtistBrowserTab::GetToolBar () const
	{
		return nullptr;
	}

	QByteArray ArtistBrowserTab::GetTabRecoverData () const
	{
		const auto& artist = Ui_.ArtistNameEdit_->text ();
		if (artist.isEmpty ())
			return {};

		QByteArray result;
		QDataStream stream (&result, QIODevice::WriteOnly);
		stream << "artistbrowser"_qba << artist;
		return result;
	}

	QIcon ArtistBrowserTab::GetTabRecoverIcon () const
	{
		return GetStaticTabClass ().Icon_;
	}

	QString ArtistBrowserTab::GetTabRecoverName () const
	{
		const auto& artist = Ui_.ArtistNameEdit_->text ();
		return artist.isEmpty () ? GetStaticTabClass ().VisibleName_ : tr ("Artist browser: %1");
	}

	void ArtistBrowserTab::DoQueries (const QString& artist)
	{
		SimilarMgr_->DefaultRequest (artist);

		auto provs = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::IArtistBioFetcher*> ();
		if (provs.isEmpty ())
		{
			static bool shownWarning = false;
			if (!shownWarning)
			{
				QMessageBox::warning (this,
						Lits::LMP,
						tr ("There aren't any plugins that can fetch biography. "
							"Check if you have installed for example the LastFMScrobble plugin."));
				shownWarning = true;
			}
			return;
		}

		BioMgr_->Request (provs.first (), artist, {});

		emit changeTabName (GetTabRecoverName ());
		emit tabRecoverDataChanged ();
	}
}
