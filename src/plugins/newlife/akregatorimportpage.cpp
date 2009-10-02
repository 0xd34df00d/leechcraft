/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "akregatorimportpage.h"
#include <QDomDocument>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NewLife
		{
			AkregatorImportPage::AkregatorImportPage (QWidget *parent)
			: QWizardPage (parent)
			{
				Ui_.setupUi (this);

				QString defaultFile = QDir::homePath () + "/.kde/share/apps/akregator/data/feeds.opml";
				if (CheckValidity (defaultFile))
					Ui_.FileLocation_->setText (defaultFile);
			}

			bool AkregatorImportPage::CheckValidity (const QString& filename) const
			{
				QFile file (filename);
				if (!file.exists () ||
						!file.open (QIODevice::ReadOnly))
					return false;

				QDomDocument document;
				if (!document.setContent (&file, true))
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

				connect (this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						wizard (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
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
				{
					QMessageBox::critical (this,
							tr ("LeechCraft"),
							tr ("The file you've selected is not a valid OPML file."));
					return;
				}

				Ui_.FileLocation_->setText (filename);
			}

			void AkregatorImportPage::handleAccepted ()
			{
				QString filename = Ui_.FileLocation_->text ();
				if (!CheckValidity (filename))
					return;

				DownloadEntity e = Util::MakeEntity (QUrl::fromLocalFile (filename),
						QString (),
						FromUserInitiated,
						QString ("text/x-opml"));
				emit gotEntity (e);
			}
		};
	};
};

