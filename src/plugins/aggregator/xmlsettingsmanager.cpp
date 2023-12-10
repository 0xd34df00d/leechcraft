/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlsettingsmanager.h"
#include <QMessageBox>
#include <QPushButton>
#include <QCoreApplication>
#include "common.h"

namespace LC::Aggregator
{
	XmlSettingsManager::XmlSettingsManager ()
	{
		Util::BaseSettingsManager::Init ();
	}

	XmlSettingsManager& XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager manager;
		return manager;
	}

	QSettings* XmlSettingsManager::BeginSettings () const
	{
		QSettings *settings =
			new QSettings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Aggregator");
		return settings;
	}

	void XmlSettingsManager::EndSettings (QSettings*) const
	{
	}

	bool ConfirmWithPersistence (const char *propName, const QString& questionMessage)
	{
		if (!XmlSettingsManager::Instance ().property (propName).toBool ())
			return true;

		QMessageBox mbox
		{
			QMessageBox::Question,
			MessageBoxTitle,
			questionMessage,
			QMessageBox::Yes | QMessageBox::No,
		};
		mbox.setDefaultButton (QMessageBox::No);

		QPushButton always { QObject::tr ("Always", "whether to remember the choice and don't ask again") };
		mbox.addButton (&always, QMessageBox::AcceptRole);

		if (mbox.exec () == QMessageBox::No)
			return false;

		if (mbox.clickedButton () == &always)
			XmlSettingsManager::Instance ().setProperty (propName, false);
		return true;
	}
}
