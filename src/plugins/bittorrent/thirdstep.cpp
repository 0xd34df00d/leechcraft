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

#include <cmath>
#include <QFileInfo>
#include <QDirIterator>
#include <QtDebug>
#include <plugininterface/util.h>
#include "thirdstep.h"
#include "secondstep.h"
#include "newtorrentwizard.h"

namespace LeechCraft
{
	namespace Plugins
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
			
				QDirIterator it (field ("RootPath").toString (), QDirIterator::Subdirectories);
				while (it.hasNext ())
				{
					QFileInfo info = it.fileInfo ();
					if (info.isFile () && info.isReadable ())
						TotalSize_ += info.size ();
					it.next ();
				}
			
				quint64 max = std::log (static_cast<long double> (TotalSize_ / 102400)) * 80;
			
				int pieceSize = 32 * 1024;
				int shouldIndex = 0;
				for (; TotalSize_ / pieceSize >= max; pieceSize *= 2, ++shouldIndex) ;
			
				if (shouldIndex > PieceSize_->count () - 1)
					shouldIndex = PieceSize_->count () - 1;
			
				PieceSize_->setCurrentIndex (shouldIndex);
			
				on_PieceSize__currentIndexChanged ();
			}
			
			void ThirdStep::on_PieceSize__currentIndexChanged ()
			{
				int mul = 32 * 1024;
				int index = PieceSize_->currentIndex ();
				while (index--)
					mul *= 2;
			
				int numPieces = TotalSize_ / mul;
				if (TotalSize_ % mul)
					++numPieces;
			
				NumPieces_->setText (QString::number (numPieces) +
						tr (" pieces (%1)")
						.arg (LeechCraft::Util::MakePrettySize (TotalSize_)));
			}
			
		};
	};
};

