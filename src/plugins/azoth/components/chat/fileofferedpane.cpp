/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fileofferedpane.h"
#include <QFileDialog>
#include <QMimeDatabase>
#include <util/sll/qtutil.h>
#include "interfaces/azoth/itransfermanager.h"
#include "../../transferjobmanager.h"
#include "../../xmlsettingsmanager.h"

namespace LC::Azoth
{
	FileOfferedPane::FileOfferedPane (const IncomingOffer& offer, TransferJobManager& transfers, QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		const auto& text = offer.Description_.isEmpty () ?
				offer.Name_ :
				"%1 (%2)"_qs.arg (offer.Name_, offer.Description_);
		Ui_.FileName_->setText (text);

		const auto& mimeType = QMimeDatabase {}.mimeTypeForFile (offer.Name_, QMimeDatabase::MatchExtension);
		if (const auto& icon = QIcon::fromTheme (mimeType.iconName ());
			!icon.isNull ())
		{
			const auto dim = Ui_.FileName_->height ();
			Ui_.FileIcon_->setPixmap (icon.pixmap (dim, dim));
		}

		connect (Ui_.AcceptButton_,
				&QPushButton::released,
				this,
				[&transfers, offer, parent]
				{
					const auto& defDir = XmlSettingsManager::Instance ().property ("DefaultXferSavePath").toString ();

					if (const QDir dir {};
						!dir.exists (defDir) && !dir.mkpath (defDir))
						qWarning () << "unable to create default path" << defDir;

					const auto& savePath = QFileDialog::getSaveFileName (parent,
							tr ("Select save path for incoming file"),
							defDir + '/' + offer.Name_);
					if (!savePath.isEmpty ())
						transfers.AcceptOffer (offer, savePath);
				});
		connect (Ui_.DeclineButton_,
				&QPushButton::released,
				this,
				[&transfers, offer] { transfers.DeclineOffer (offer); });

		connect (&transfers,
				&TransferJobManager::jobNoLongerOffered,
				this,
				[this, offer] (const IncomingOffer& deoffered)
				{
					if (offer == deoffered)
						deleteLater ();
				});
	}
}
