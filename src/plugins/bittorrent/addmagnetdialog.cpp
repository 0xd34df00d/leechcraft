/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addmagnetdialog.h"
#include <optional>
#include <QClipboard>
#include <QFileDialog>
#include <QUrl>
#include <QUrlQuery>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "xmlsettingsmanager.h"

namespace LC::BitTorrent
{
	namespace
	{
		bool IsMagnet (const QString& link)
		{
			const auto& url = QUrl::fromUserInput (link);
			if (!url.isValid () || url.scheme () != QStringLiteral ("magnet"))
				return false;

			const auto& items = QUrlQuery { url }.queryItems ();
			return std::any_of (items.begin (), items.end (),
					[] (const auto& item)
					{
						return item.first == QStringLiteral ("xt") &&
								item.second.startsWith (QStringLiteral ("urn:btih:"));
					});
		}

		std::optional<QString> CheckClipboard (QClipboard::Mode mode)
		{
			const auto& text = qApp->clipboard ()->text (mode);
			return IsMagnet (text) ? text : std::optional<QString> {};
		}
	}

	AddMagnetDialog::AddMagnetDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		auto magnet = CheckClipboard (QClipboard::Clipboard);
		if (!magnet)
			magnet = CheckClipboard (QClipboard::Selection);
		if (magnet)
			Ui_.Magnet_->setText (*magnet);

		const auto& dir = XmlSettingsManager::Instance ()->property ("LastSaveDirectory").toString ();
		Ui_.SavePath_->setText (dir);

		const auto checkComplete = [this]
		{
			const auto isComplete = !Ui_.SavePath_->text ().isEmpty () &&
					IsMagnet (Ui_.Magnet_->text ());
			Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isComplete);
		};
		checkComplete ();
		connect (Ui_.SavePath_,
				&QLineEdit::textChanged,
				checkComplete);
		connect (Ui_.Magnet_,
				&QLineEdit::textChanged,
				checkComplete);

		connect (Ui_.BrowseButton_,
				&QPushButton::released,
				[this]
				{
					const auto& dir = QFileDialog::getExistingDirectory (this,
							tr ("Select save directory"),
							Ui_.SavePath_->text (),
							{});
					if (dir.isEmpty ())
						return;

					XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
					Ui_.SavePath_->setText (dir);
				});
	}

	QString AddMagnetDialog::GetLink () const
	{
		return Ui_.Magnet_->text ();
	}

	QString AddMagnetDialog::GetPath () const
	{
		return Ui_.SavePath_->text ();
	}

	QStringList AddMagnetDialog::GetTags () const
	{
		return GetProxyHolder ()->GetTagsManager ()->SplitToIDs (Ui_.Tags_->text ());
	}
}
