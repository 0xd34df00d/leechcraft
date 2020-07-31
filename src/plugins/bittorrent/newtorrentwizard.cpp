/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "newtorrentwizard.h"
#include "intropage.h"
#include "firststep.h"
#include "secondstep.h"
#include "thirdstep.h"

namespace LC
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
}
}
