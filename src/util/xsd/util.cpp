/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QDialogButtonBox>
#include <QDialog>
#include <QVBoxLayout>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC
{
namespace Util
{
	XmlSettingsDialog* OpenXSD (const QString& title, const QString& filename, Util::BaseSettingsManager *bsm)
	{
		auto lay = new QVBoxLayout;

		auto xsd = new Util::XmlSettingsDialog;
		xsd->RegisterObject (bsm, filename);
		lay->addWidget (xsd->GetWidget ());

		auto bbox = new QDialogButtonBox { QDialogButtonBox::Ok | QDialogButtonBox::Cancel };
		lay->addWidget (bbox);

		auto dia = new QDialog;
		dia->setLayout (lay);

		QObject::connect (bbox,
				SIGNAL (accepted ()),
				xsd,
				SLOT (accept ()));
		QObject::connect (bbox,
				SIGNAL (rejected ()),
				xsd,
				SLOT (reject ()));
		QObject::connect (bbox,
				SIGNAL (accepted ()),
				dia,
				SLOT (accept ()));
		QObject::connect (bbox,
				SIGNAL (rejected ()),
				dia,
				SLOT (reject ()));

		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->setWindowTitle (title);
		dia->show ();

		return xsd;
	}
}
}
