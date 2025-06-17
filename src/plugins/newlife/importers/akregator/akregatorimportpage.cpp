/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "akregatorimportpage.h"
#include <QDomDocument>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <util/xpc/util.h>

namespace LC
{
namespace NewLife
{
namespace Importers
{
	AkregatorImportPage::AkregatorImportPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: EntityGeneratingPage { proxy, parent }
	{
		Ui_.setupUi (this);
		Ui_.ImportSettings_->setText (Ui_.ImportSettings_->text ().arg ("Akregator"));

		setTitle (tr ("Akregator's feeds import"));
		setSubTitle (tr ("Select Akregator's feeds file and options"));
	}

	bool AkregatorImportPage::CheckValidity (const QString& filename) const
	{
		QFile file (filename);
		if (!file.exists () ||
				!file.open (QIODevice::ReadOnly))
			return false;

		QDomDocument document;
		if (!document.setContent (&file, QDomDocument::ParseOption::UseNamespaceProcessing))
			return false;

		QDomElement root = document.documentElement ();
		if (root.tagName () != "opml")
			return false;

		QDomNodeList heads = root.elementsByTagName ("head");
		if (heads.size () != 1 || !heads.at (0).isElement ())
			return false;

		QDomNodeList bodies = root.elementsByTagName ("body");
		if (bodies.size () != 1 || !bodies.at (0).isElement ())
			return false;

		if (!bodies.at (0).toElement ().elementsByTagName ("outline").size ())
			return false;

		return true;
	}

	bool AkregatorImportPage::isComplete () const
	{
		return CheckValidity (Ui_.FileLocation_->text ());
	}

	int AkregatorImportPage::nextId () const
	{
		return -1;
	}

	void AkregatorImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));

		QString defaultFile = QDir::homePath () + "/.kde/share/apps/akregator/data/feeds.opml";
		if (CheckValidity (defaultFile))
			Ui_.FileLocation_->setText (defaultFile);
	}

	void AkregatorImportPage::on_Browse__released ()
	{
		QString filename = QFileDialog::getOpenFileName (this,
				tr ("Select Akregator's OPML file"),
				QDir::homePath () + "/.kde/share/apps/akregator/data",
				tr ("OPML files (*.opml *.xml);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		if (!CheckValidity (filename))
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("The file you've selected is not a valid OPML file."));
		else
			Ui_.FileLocation_->setText (filename);

		emit completeChanged ();
	}

	void AkregatorImportPage::on_FileLocation__textEdited (const QString&)
	{
		emit completeChanged ();
	}

	void AkregatorImportPage::handleAccepted ()
	{
		QString filename = Ui_.FileLocation_->text ();
		if (!CheckValidity (filename))
			return;

		Entity e = Util::MakeEntity (QUrl::fromLocalFile (filename),
				QString (),
				FromUserInitiated,
				"text/x-opml");

		if (Ui_.ImportSettings_->checkState () == Qt::Checked)
		{
			QSettings settings (QDir::homePath () + "/.kde/share/config/akregatorrc",
					QSettings::IniFormat);
			if (settings.status () == QSettings::NoError)
			{
				if (settings.contains ("Show Tray Icon"))
					e.Additional_ ["ShowTrayIcon"] = settings.value ("Show Tray Icon");
				if (settings.contains ("Fetch On Startup"))
					e.Additional_ ["UpdateOnStartup"] = settings.value ("Fetch On Startup");
				if (settings.contains ("Auto Fetch Interval"))
					e.Additional_ ["UpdateTimeout"] = settings.value ("Auto Fetch Interval");

				settings.beginGroup ("Archive");
				if (settings.contains ("Max Article Number"))
					e.Additional_ ["MaxArticles"] = settings.value ("Max Article Number");
				if (settings.contains ("Max Article Age"))
					e.Additional_ ["MaxAge"] = settings.value ("Max Article Age");
				settings.endGroup ();

				e.Additional_ ["UserVisibleName"] = tr ("Akregator settings");
			}
			else
				QMessageBox::critical (0,
						"LeechCraft",
						tr ("Could not access or parse Akregator settings."));
		}

		SendEntity (e);
	}
}
}
}
