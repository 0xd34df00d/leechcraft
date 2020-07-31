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

namespace LC
{
namespace BitTorrent
{
	AddMultipleTorrents::AddMultipleTorrents (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.OpenDirectory_->setText (XmlSettingsManager::Instance ()->property ("LastTorrentDirectory").toString ());
		Ui_.SaveDirectory_->setText (XmlSettingsManager::Instance ()->property ("LastSaveDirectory").toString ());

		new Util::TagsCompleter (Ui_.TagsEdit_);
		Ui_.TagsEdit_->AddSelector ();
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

	void AddMultipleTorrents::on_BrowseOpen__released ()
	{
		const auto& dir = QFileDialog::getExistingDirectory (this,
				tr ("Select directory with torrents"),
				Ui_.OpenDirectory_->text ());
		if (dir.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory", dir);
		Ui_.OpenDirectory_->setText (dir);
	}

	void AddMultipleTorrents::on_BrowseSave__released ()
	{
		const auto& dir = QFileDialog::getExistingDirectory (this,
				tr ("Select save directory"),
				Ui_.SaveDirectory_->text ());
		if (dir.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
		Ui_.SaveDirectory_->setText (dir);
	}
}
}
