/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QSettings>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC::Util
{
	XmlSettingsDialog* OpenXSD (const QString& title, const QString& filename, Util::BaseSettingsManager *bsm)
	{
		auto lay = new QVBoxLayout;

		auto xsd = new XmlSettingsDialog;
		xsd->RegisterObject (bsm, filename);
		lay->addWidget (xsd->GetWidget ());

		auto bbox = new QDialogButtonBox { QDialogButtonBox::Ok | QDialogButtonBox::Cancel };
		lay->addWidget (bbox);

		auto dia = new QDialog;
		dia->setLayout (lay);

		QObject::connect (bbox,
				&QDialogButtonBox::accepted,
				xsd,
				&XmlSettingsDialog::accept);
		QObject::connect (bbox,
				&QDialogButtonBox::rejected,
				xsd,
				&XmlSettingsDialog::reject);
		QObject::connect (bbox,
				&QDialogButtonBox::accepted,
				dia,
				&QDialog::accept);
		QObject::connect (bbox,
				&QDialogButtonBox::rejected,
				dia,
				&QDialog::reject);

		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->setWindowTitle (title);
		dia->show ();

		return xsd;
	}

	UTIL_XSD_API std::shared_ptr<QSettings> MakeGroupSettings (const QString& suffix, const QString& groupName)
	{
		std::shared_ptr<QSettings> settings
		{
			new QSettings
			{
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + '_' + suffix
			},
			[] (QSettings *settings)
			{
				settings->endGroup ();
				delete settings;
			}
		};
		settings->beginGroup (groupName);
		return settings;
	}
}
