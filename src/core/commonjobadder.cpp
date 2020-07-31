/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QClipboard>
#include <QFileDialog>
#include <QDir>
#include "commonjobadder.h"
#include "xmlsettingsmanager.h"

using namespace LC;

LC::CommonJobAdder::CommonJobAdder (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
	What_->setPlaceholderText (What_->toolTip ());
	const QString &text = XmlSettingsManager::Instance ()->
			Property ("LastWhatFolder", QString ()).toString ();
	if (!text.isEmpty ())
		What_->setText (text);
}

QString LC::CommonJobAdder::GetString () const
{
	return What_->text ();
}

void LC::CommonJobAdder::on_Browse__released ()
{
	const QString &name = QFileDialog::getOpenFileName (this,
			tr ("Select file"),
			XmlSettingsManager::Instance ()->Property ("LastWhatFolder",
				QDir::homePath ()).toString ());
	if (name.isEmpty ())
		return;

	What_->setText (name);
	XmlSettingsManager::Instance ()->setProperty ("LastWhatFolder", name);
}
