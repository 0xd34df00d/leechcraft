/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiotracksgrabdialog.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <util/sll/prelude.h>
#include <util/sll/slotclosure.h>
#include <util/lmp/util.h>
#include "mediainfo.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		QString GetDefaultPath ()
		{
			return QStandardPaths::writableLocation (QStandardPaths::DownloadLocation);
		}
	}

	RadioTracksGrabDialog::RadioTracksGrabDialog (const QList<Media::AudioInfo>& infos, QWidget *parent)
	: RadioTracksGrabDialog { Util::Map (infos, &MediaInfo::FromAudioInfo), parent }
	{
	}

	RadioTracksGrabDialog::RadioTracksGrabDialog (const QList<MediaInfo>& infos, QWidget *parent)
	: QDialog { parent }
	, NamesPreviewModel_ { new QStandardItemModel { this } }
	{
		NamesPreviewModel_->setHorizontalHeaderLabels ({ tr ("Artist"), tr ("Title"), tr ("File name") });
		for (const auto& info : infos)
		{
			const QList<QStandardItem*> items
			{
				new QStandardItem { info.Artist_ },
				new QStandardItem { info.Title_ },
				new QStandardItem {}
			};

			for (auto item : items)
				item->setEditable (false);

			NamesPreviewModel_->appendRow (items);
		}

		Ui_.setupUi (this);

		const auto& helpText = tr ("The following variables are allowed in the pattern: %1.")
				.arg (GetSubstGettersKeys ().join ("; "));
		Ui_.NameMask_->setToolTip (helpText);

		Ui_.Destination_->setText (XmlSettingsManager::Instance ()
					.Property ("LastTracksGrabPath", GetDefaultPath ()).toString ());
		Ui_.NamesPreview_->setModel (NamesPreviewModel_);

		const auto subster = [this, infos]
			{
				auto patternText = Ui_.NameMask_->text ();
				if (!patternText.endsWith (".mp3", Qt::CaseInsensitive))
					patternText += ".mp3";

				Names_ = PerformSubstitutions (patternText, infos,
						[this] (int row, const QString& name)
							{ NamesPreviewModel_->item (row, 2)->setText (name); });
			};

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			subster,
			Ui_.NameMask_,
			SIGNAL (textChanged (QString)),
			this
		};
		subster ();

		connect (Ui_.Destination_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (checkCompleteness ()));
		connect (Ui_.NameMask_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (checkCompleteness ()));
		checkCompleteness ();
	}

	const QStringList& RadioTracksGrabDialog::GetNames () const
	{
		return Names_;
	}

	QString RadioTracksGrabDialog::GetDestination () const
	{
		return Ui_.Destination_->text ();
	}

	QString RadioTracksGrabDialog::SelectDestination (QString dir, QWidget *parent)
	{
		if (dir.isEmpty ())
			dir = XmlSettingsManager::Instance ()
					.Property ("LastTracksGrabPath", GetDefaultPath ()).toString ();

		const auto& path = QFileDialog::getExistingDirectory (parent,
				tr ("Select tracks save directory"),
				dir);

		if (!path.isEmpty ())
			XmlSettingsManager::Instance ().setProperty ("LastTracksGrabPath", path);

		return path;
	}

	bool RadioTracksGrabDialog::IsComplete () const
	{
		QFileInfo toInfo { GetDestination () };
		if (!toInfo.exists () || !toInfo.isDir () || !toInfo.isWritable ())
			return false;

		if (Names_.isEmpty ())
			return false;

		if (std::any_of (Names_.begin (), Names_.end (),
				[] (const QString& name) { return name.isEmpty (); }))
			return false;

		auto uniqueNames = Names_;
		if (uniqueNames.removeDuplicates ())
			return false;

		return true;
	}

	void RadioTracksGrabDialog::on_Browse__released ()
	{
		const auto& path = SelectDestination (Ui_.Destination_->text (), this);
		if (path.isEmpty ())
			return;

		Ui_.Destination_->setText (path);
	}

	void RadioTracksGrabDialog::checkCompleteness ()
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (IsComplete ());
	}
}
}
