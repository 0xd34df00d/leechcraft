/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include <QLabel>
#include <QVBoxLayout>
#include "intropage.h"

namespace LeechCraft
{
	namespace Plugins
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
		};
	};
};

