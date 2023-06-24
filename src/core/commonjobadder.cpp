/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "commonjobadder.h"
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <interfaces/structures.h>
#include <util/xpc/util.h>
#include "entitymanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
	CommonJobAdder::CommonJobAdder (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
		Ui_.What_->setPlaceholderText (Ui_.What_->toolTip ());
		const auto& text = XmlSettingsManager::Instance ()->Property ("LastWhatFolder", {}).toString ();
		if (!text.isEmpty ())
			Ui_.What_->setText (text);

		setAttribute (Qt::WA_DeleteOnClose);

		connect (Ui_.Browse_,
				&QPushButton::released,
				this,
				&CommonJobAdder::Browse);
		connect (this,
				&QDialog::accepted,
				this,
				&CommonJobAdder::AddJob);
	}

	void CommonJobAdder::Browse ()
	{
		const auto& name = QFileDialog::getOpenFileName (this,
				tr ("Select file"),
				XmlSettingsManager::Instance ()->Property ("LastWhatFolder",
						QDir::homePath ()).toString ());
		if (name.isEmpty ())
			return;

		Ui_.What_->setText (name);
		XmlSettingsManager::Instance ()->setProperty ("LastWhatFolder", name);
	}

	void CommonJobAdder::AddJob ()
	{
		const auto& what = Ui_.What_->text ();
		if (what.isEmpty ())
			return;

		Entity e;
		if (QFile::exists (what))
			e.Entity_ = QUrl::fromLocalFile (what);
		else
		{
			const QUrl url { what };
			e.Entity_ = url.isValid () ? url : what;
		}
		e.Parameters_ = FromUserInitiated;

		if (!EntityManager { nullptr, nullptr }.HandleEntity (e))
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("No plugins are able to download \"%1\"").arg (what));
	}
}
