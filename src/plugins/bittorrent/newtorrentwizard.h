/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>
#include "newtorrentparams.h"

namespace LC
{
namespace BitTorrent
{
	class NewTorrentWizard : public QWizard
	{
		Q_OBJECT
	public:
		enum Page { PageIntro
			, PageFirstStep
			, PageSecondStep
			, PageThirdStep };

		NewTorrentWizard (QWidget *parent = 0);
		virtual void accept ();
		NewTorrentParams GetParams () const;
	};
}
}
