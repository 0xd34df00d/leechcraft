/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "movetorrentfiles.h"
#include <QFileDialog>
#include "xmlsettingsmanager.h"

namespace LC::BitTorrent
{
	MoveTorrentFiles::MoveTorrentFiles (QStringList oldDirectories, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		oldDirectories.removeDuplicates ();

		if (oldDirectories.size () == 1)
			Ui_.OldLocation_->setText (oldDirectories.front ());
		else
		{
			Ui_.OldLocation_->setToolTip (oldDirectories.join (QStringLiteral (", ")));
			Ui_.OldLocation_->setPlaceholderText (tr ("Multiple directories"));
		}

		auto& xsm = XmlSettingsManager::Instance ();
		const auto& saveDirectory = xsm.property ("LastSaveDirectory").toString ();
		const auto& moveDirectory = xsm.Property ("LastMoveDirectory", saveDirectory).toString ();

		Ui_.NewLocation_->setText (moveDirectory);

		connect (Ui_.Browse_,
				&QPushButton::released,
				[this]
				{
					const auto& dir = QFileDialog::getExistingDirectory (this,
							tr ("New location"),
							Ui_.NewLocation_->text ());
					if (dir.isEmpty ())
						return;

					Ui_.NewLocation_->setText (dir);
				});

		connect (this,
				&QDialog::accepted,
				this,
				[this] { XmlSettingsManager::Instance ().setProperty ("LastMoveDirectory", GetNewLocation ()); });
	}

	QString MoveTorrentFiles::GetNewLocation () const
	{
		return Ui_.NewLocation_->text ();
	}
}
