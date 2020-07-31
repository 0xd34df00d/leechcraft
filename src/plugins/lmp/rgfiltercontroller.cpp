/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rgfiltercontroller.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QtDebug>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/xsd/util.h>
#include "util/lmp/filtersettingsmanager.h"
#include "engine/path.h"
#include "engine/sourceobject.h"
#include "localcollectionstorage.h"

namespace LC
{
namespace LMP
{
	RGFilterController::RGFilterController (RGFilter *filter, Path *path)
	: RGFilter_ { filter }
	, Path_ { path }
	, FSM_ { new FilterSettingsManager { "ReplayGain", this } }
	{
		const QList<QByteArray> rgProps
		{
			"RGAlbumMode",
			"RGLimiting",
			"RGPreamp"
		};
		FSM_->RegisterObject (rgProps, this, "setRG");
		setRG ();

		const auto srcObj = path->GetSourceObject ();
		connect (srcObj,
				SIGNAL (currentSourceChanged (AudioSource)),
				this,
				SLOT (updateRGData (AudioSource)));
		updateRGData (srcObj->GetCurrentSource ());
	}

	void RGFilterController::OpenDialog ()
	{
		Util::OpenXSD (tr ("ReplayGain configuration"), "lmpfilterrgsettings.xml", FSM_);
	}

	void RGFilterController::setRG ()
	{
		RGFilter_->SetAlbumMode (FSM_->property ("RGAlbumMode").toBool ());
		RGFilter_->SetPreamp (FSM_->property ("RGPreamp").toDouble ());
		RGFilter_->SetLimiterEnabled (FSM_->property ("RGLimiting").toBool ());
	}

	void RGFilterController::updateRGData (const AudioSource& source)
	{
		if (!source.IsLocalFile ())
		{
			RGFilter_->SetRG ({});
			return;
		}

		LocalCollectionStorage storage;
		const auto& data = storage.GetRgTrackInfo (source.GetLocalPath ());
		RGFilter_->SetRG (data);
	}
}
}
