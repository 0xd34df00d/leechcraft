/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "similarview.h"
#include <QQmlEngine>
#include <interfaces/core/icoreproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/standardnamfactory.h>
#include <util/sys/paths.h>
#include "similarviewmanager.h"

namespace LC
{
namespace LMP
{
	SimilarView::SimilarView (QWidget *parent)
	: QQuickWidget (parent)
	, Manager_ (new SimilarViewManager (this, this))
	{
		setResizeMode (SizeRootObjectToView);
		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (GetProxyHolder ()));

		new Util::StandardNAMFactory ("lmp/qml",
				[] { return 50 * 1024 * 1024; },
				engine ());

		setSource (Util::GetSysPathUrl (Util::SysPath::QML, "lmp", "SimilarView.qml"));
		Manager_->InitWithSource ();

		setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	void SimilarView::SetSimilarArtists (Media::SimilarityInfos_t infos)
	{
		Manager_->SetInfos (infos);
	}
}
}
