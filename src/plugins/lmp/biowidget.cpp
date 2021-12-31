/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "biowidget.h"
#include <QQuickWidget>
#include <util/util.h>
#include <util/qml/standardnamfactory.h>
#include <util/sys/paths.h>
#include <util/sll/udls.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iinfo.h>
#include "xmlsettingsmanager.h"
#include "bioviewmanager.h"
#include "literals.h"

namespace LC::LMP
{
	BioWidget::BioWidget (QWidget *parent)
	: QWidget { parent }
	, View_ { new QQuickWidget }
	, Manager_ { new BioViewManager { View_, this } }
	{
		Ui_.setupUi (this);

		View_->setResizeMode (QQuickWidget::SizeRootObjectToView);
		View_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

		layout ()->addWidget (View_);

		new Util::StandardNAMFactory (Lits::LmpSlashQml,
				[] { return 50_mib; },
				View_->engine ());

		View_->setSource (Util::GetSysPathUrl (Util::SysPath::QML,
				Lits::LmpQmlSubdir,
				QStringLiteral ("BioView.qml")));
		Manager_->InitWithSource ();

		const auto& lastProv = XmlSettingsManager::Instance ()
				.Property ("LastUsedBioProvider", QString ()).toString ();

		auto providerObjs = GetProxyHolder ()->GetPluginsManager ()->
				GetAllCastableRoots<Media::IArtistBioFetcher*> ();
		for (auto providerObj : providerObjs)
		{
			const auto provider = qobject_cast<Media::IArtistBioFetcher*> (providerObj);

			Providers_ << provider;

			const auto& icon = qobject_cast<IInfo*> (providerObj)->GetIcon ();
			Ui_.Provider_->addItem (icon, provider->GetServiceName ());
			if (lastProv == provider->GetServiceName ())
				Ui_.Provider_->setCurrentIndex (Ui_.Provider_->count () - 1);
		}

		connect (Ui_.Provider_,
				qOverload<int> (&QComboBox::currentIndexChanged),
				[this]
				{
					RequestBiography ();
					SaveLastUsedProv ();
				});

		connect (Manager_,
				&BioViewManager::gotArtistImage,
				this,
				&BioWidget::gotArtistImage);
	}

	void BioWidget::SetCurrentArtist (const QString& artist, const QStringList& hints)
	{
		if (artist.isEmpty ())
			return;

		if (artist == Current_.Artist_ && hints == Current_.Hints_)
			return;

		Current_ = { artist, hints };

		RequestBiography ();
	}

	void BioWidget::SaveLastUsedProv ()
	{
		const int idx = Ui_.Provider_->currentIndex ();
		const auto& prov = idx >= 0 ?
				Providers_.value (idx)->GetServiceName () :
				QString ();

		XmlSettingsManager::Instance ().setProperty ("LastUsedBioProvider", prov);
	}

	void BioWidget::RequestBiography ()
	{
		const int idx = Ui_.Provider_->currentIndex ();
		if (idx < 0 || Current_.Artist_.isEmpty ())
			return;

		Manager_->Request (Providers_ [idx], Current_.Artist_, Current_.Hints_);
	}
}
