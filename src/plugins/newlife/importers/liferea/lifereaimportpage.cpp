/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lifereaimportpage.h"
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
	LifereaImportPage::LifereaImportPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: EntityGeneratingPage (proxy, parent)
	{
		Ui_.setupUi (this);
		Ui_.ImportSettings_->setText (Ui_.ImportSettings_->text ().arg ("Liferea"));

		setTitle (tr ("Liferea's feeds import"));
		setSubTitle (tr ("Select Liferea's base location and options"));
	}

	bool LifereaImportPage::CheckValidity (const QString& directory) const
	{
		if (!QFile::exists (directory + "/liferea.db"))
			return false;

		QString filename = directory + "/feedlist.opml";
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

	bool LifereaImportPage::isComplete () const
	{
		return CheckValidity (Ui_.FileLocation_->text ());
	}

	int LifereaImportPage::nextId () const
	{
		return -1;
	}

	void LifereaImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));

		QString defaultFile = GetSuggestion ();

		if (CheckValidity (defaultFile))
			Ui_.FileLocation_->setText (defaultFile);
	}

	void LifereaImportPage::on_Browse__released ()
	{
		QString filename = QFileDialog::getExistingDirectory (this,
				tr ("Select Liferea's directory"),
				GetSuggestion ());

		if (filename.isEmpty ())
			return;

		if (!CheckValidity (filename))
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("The directory you've selected is not Liferea's directory."));
		else
			Ui_.FileLocation_->setText (filename);

		emit completeChanged ();
	}

	void LifereaImportPage::on_FileLocation__textEdited (const QString&)
	{
		emit completeChanged ();
	}

	void LifereaImportPage::handleAccepted ()
	{
		QString filename = Ui_.FileLocation_->text ();
		if (!CheckValidity (filename))
			return;

		Entity e = Util::MakeEntity (QUrl::fromLocalFile (filename + "/feedlist.opml"),
				QString (),
				FromUserInitiated,
				"text/x-opml");

		SendEntity (e);
	}

	QString LifereaImportPage::GetSuggestion () const
	{
		QDir home = QDir::home ();
		QStringList entries = home.entryList (QStringList (".liferea_*"),
				QDir::Dirs | QDir::Hidden,
				QDir::Name);

		QString defaultFile;
		if (entries.size ())
			defaultFile = QDir::homePath () + "/" + entries.last ();

		return defaultFile;
	}
}
}
}
