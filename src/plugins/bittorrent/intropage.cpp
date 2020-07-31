/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QLabel>
#include <QVBoxLayout>
#include "intropage.h"

namespace LC
{
namespace BitTorrent
{
	IntroPage::IntroPage (QWidget *parent)
	: QWizardPage (parent)
	{
		setTitle (tr ("Introduction"));
		Label_ = new QLabel (tr ("This wizard will generate a torrent file. "
									"You simply need so specify the torrent "
									"name, files to include and optionally few "
									"other options to produce your torrent file."));
		Label_->setWordWrap (true);
		QVBoxLayout *lay = new QVBoxLayout;
		lay->addWidget (Label_);
		setLayout (lay);
	}
}
}
