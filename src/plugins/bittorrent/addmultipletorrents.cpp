/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QFileDialog>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <util/tags/tagscompleter.h>
#include "addmultipletorrents.h"
#include "xmlsettingsmanager.h"

namespace LC::BitTorrent
{
	AddMultipleTorrents::AddMultipleTorrents (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		auto& xsm = XmlSettingsManager::Instance ();
		Ui_.OpenDirectory_->setText (xsm.property ("LastTorrentDirectory").toString ());
		Ui_.SaveDirectory_->setText (xsm.property ("LastSaveDirectory").toString ());

		new Util::TagsCompleter (Ui_.TagsEdit_);
		Ui_.TagsEdit_->AddSelector ();

		const auto chooser = [this, &xsm] (const QString& header, QLineEdit *edit, const char *propName)
		{
			return [=, this, &xsm]
			{
				const auto& dir = QFileDialog::getExistingDirectory (this,
						header,
						edit->text ());
				if (dir.isEmpty ())
					return;

				xsm.setProperty (propName, dir);
				edit->setText (dir);
			};
		};

		connect (Ui_.BrowseOpen_,
				&QPushButton::released,
				chooser (tr ("Select directory with torrents"), Ui_.OpenDirectory_, "LastTorrentDirectory"));
		connect (Ui_.BrowseSave_,
				&QPushButton::released,
				chooser (tr ("Select save directory"), Ui_.SaveDirectory_, "LastSaveDirectory"));
	}

	QString AddMultipleTorrents::GetOpenDirectory () const
	{
		return Ui_.OpenDirectory_->text ();
	}

	QString AddMultipleTorrents::GetSaveDirectory () const
	{
		return Ui_.SaveDirectory_->text ();
	}

	QStringList AddMultipleTorrents::GetTags () const
	{
		return GetProxyHolder ()->GetTagsManager ()->SplitToIDs (Ui_.TagsEdit_->text ());
	}

	bool AddMultipleTorrents::ShouldAddAsStarted () const
	{
		return Ui_.AddAsStarted_->isChecked ();
	}

	bool AddMultipleTorrents::OnlyIfExists () const
	{
		return Ui_.OnlyIfExists_->isChecked ();
	}
}
