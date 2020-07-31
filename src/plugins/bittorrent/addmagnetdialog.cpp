/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addmagnetdialog.h"
#include <QClipboard>
#include <QFileDialog>
#include <QUrl>
#include <QUrlQuery>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace BitTorrent
{
	namespace
	{
		bool IsMagnet (const QString& link)
		{
			const auto& url = QUrl::fromUserInput (link);
			if (!url.isValid () || url.scheme () != "magnet")
				return false;

			const auto& items = QUrlQuery { url }.queryItems ();
			return std::any_of (items.begin (), items.end (),
					[] (const auto& item) { return item.first == "xt" && item.second.startsWith ("urn:btih:"); });
		}

		QString CheckClipboard (QClipboard::Mode mode)
		{
			const auto& text = qApp->clipboard ()->text (mode);
			return IsMagnet (text) ? text : QString {};
		}
	}

	AddMagnetDialog::AddMagnetDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		auto text = CheckClipboard (QClipboard::Clipboard);
		if (text.isEmpty ())
			text = CheckClipboard (QClipboard::Selection);

		if (!text.isEmpty ())
			Ui_.Magnet_->setText (text);

		const auto& dir = XmlSettingsManager::Instance ()->
				property ("LastSaveDirectory").toString ();
		Ui_.SavePath_->setText (dir);

		checkComplete ();
		connect (Ui_.SavePath_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (checkComplete ()));
		connect (Ui_.Magnet_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (checkComplete ()));
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
		auto tm = Core::Instance ()->GetProxy ()->GetTagsManager ();

		QStringList result;
		for (const auto& tag : tm->Split (Ui_.Tags_->text ()))
			result << tm->GetID (tag);
		return result;
	}

	void AddMagnetDialog::on_BrowseButton__released()
	{
		const auto& dir = QFileDialog::getExistingDirectory (this,
				tr ("Select save directory"),
				Ui_.SavePath_->text (),
				{});
		if (dir.isEmpty ())
			return;

		XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
		Ui_.SavePath_->setText (dir);
	}

	void AddMagnetDialog::checkComplete ()
	{
		const auto isComplete = !Ui_.SavePath_->text ().isEmpty () &&
				IsMagnet (Ui_.Magnet_->text ());
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (isComplete);
	}
}
}
