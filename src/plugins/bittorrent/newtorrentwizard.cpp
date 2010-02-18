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

#include "newtorrentwizard.h"
#include "intropage.h"
#include "firststep.h"
#include "secondstep.h"
#include "thirdstep.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			NewTorrentWizard::NewTorrentWizard (QWidget *parent)
			: QWizard (parent)
			{
				setWindowTitle (tr ("New torrent wizard"));
				setWizardStyle (QWizard::ModernStyle);
			
				setPage (PageIntro, new IntroPage);
				setPage (PageFirstStep, new FirstStep);
				setPage (PageSecondStep, new ThirdStep);
			}
			
			void NewTorrentWizard::accept ()
			{
				QWizard::accept ();
			}
			
			NewTorrentParams NewTorrentWizard::GetParams () const
			{
				NewTorrentParams result;
			
				result.Output_ = field ("Output").toString ();
				result.AnnounceURL_ = field ("AnnounceURL").toString ();
				result.Date_ = field ("Date").toDate ();
				result.Comment_ = field ("Comment").toString ();
				result.Path_ = field ("RootPath").toString ();
				result.URLSeeds_ = field ("URLSeeds").toString ().split (QRegExp("\\s+"));
				result.DHTEnabled_ = field ("DHTEnabled").toBool ();
				result.DHTNodes_ = field ("DHTNodes").toString ().split (QRegExp("\\s+"));
				result.PieceSize_ = 32 * 1024;
				int index = field ("PieceSize").toInt ();
				while (index--)
					result.PieceSize_ *= 2;
			
				if (result.Path_.endsWith ('/'))
					result.Path_.remove (result.Path_.size () - 1, 1);
			
				return result;
			}
			
		};
	};
};

