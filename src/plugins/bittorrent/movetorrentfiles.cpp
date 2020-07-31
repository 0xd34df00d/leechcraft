/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "movetorrentfiles.h"
#include <QFileDialog>
#include <xmlsettingsmanager.h>

namespace LC
{
namespace BitTorrent
{
	MoveTorrentFiles::MoveTorrentFiles (QStringList oldDirectories, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		Q_ASSERT (!oldDirectories.empty ());

		oldDirectories.removeDuplicates ();

		if (1 == oldDirectories.size ())
			Ui_.OldLocation_->setText (oldDirectories.front ());
		else
		{
			Ui_.OldLocation_->setToolTip (oldDirectories.join (", "));
			Ui_.OldLocation_->setPlaceholderText (tr ("Multiple directories"));
		}

		const auto xsm = XmlSettingsManager::Instance ();
		const auto& moveDirectory = xsm->Property ("LastMoveDirectory",
					xsm->property ("LastSaveDirectory").toString ()).toString ();

		Ui_.NewLocation_->setText (moveDirectory);
	}

	QString MoveTorrentFiles::GetNewLocation () const
	{
		return Ui_.NewLocation_->text ();
	}

	void MoveTorrentFiles::on_Browse__released ()
	{
		const auto& dir = QFileDialog::getExistingDirectory (this, tr ("New location"),
				Ui_.NewLocation_->text ());
		if (dir.isEmpty () || dir == Ui_.NewLocation_->text ())
			return;
		Ui_.NewLocation_->setText (dir);
	}
}
}
