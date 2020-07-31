/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <cmath>
#include <QFileInfo>
#include <QDirIterator>
#include <QtDebug>
#include <util/util.h>
#include "thirdstep.h"
#include "secondstep.h"
#include "newtorrentwizard.h"

namespace LC
{
namespace BitTorrent
{
	ThirdStep::ThirdStep (QWidget *parent)
	: QWizardPage (parent)
	, TotalSize_ (0)
	{
		setupUi (this);
		registerField ("PieceSize", PieceSize_);
		registerField ("URLSeeds", URLSeeds_);
		registerField ("DHTEnabled", DHTEnabled_);
		registerField ("DHTNodes", DHTNodes_);
	}

	void ThirdStep::initializePage ()
	{
		TotalSize_ = 0;
		QString path = field ("RootPath").toString ();

		QFileInfo pathInfo (path);
		if (pathInfo.isDir ())
		{
			QDirIterator it (path,
					QDirIterator::Subdirectories);
			while (it.hasNext ())
			{
				it.next ();
				QFileInfo info = it.fileInfo ();
				if (info.isFile () && info.isReadable ())
					TotalSize_ += info.size ();
			}
		}
		else if (pathInfo.isFile () &&
				pathInfo.isReadable ())
			TotalSize_ += pathInfo.size ();

		quint64 max = std::log (static_cast<long double> (TotalSize_ / 102400)) * 80;

		quint32 pieceSize = 32 * 1024;
		int shouldIndex = 0;
		for (; TotalSize_ / pieceSize >= max; pieceSize *= 2, ++shouldIndex) ;

		if (shouldIndex > PieceSize_->count () - 1)
			shouldIndex = PieceSize_->count () - 1;

		PieceSize_->setCurrentIndex (shouldIndex);

		on_PieceSize__currentIndexChanged ();
	}

	void ThirdStep::on_PieceSize__currentIndexChanged ()
	{
		quint32 mul = 32 * 1024;
		int index = PieceSize_->currentIndex ();
		while (index--)
			mul *= 2;

		int numPieces = TotalSize_ / mul;
		if (TotalSize_ % mul)
			++numPieces;

		NumPieces_->setText (QString::number (numPieces) +
				tr (" pieces (%1)")
				.arg (LC::Util::MakePrettySize (TotalSize_)));
	}
}
}
