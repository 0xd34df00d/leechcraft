/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include <QFileDialog>
#include <plugininterface/proxy.h>
#include "secondstep.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			SecondStep::SecondStep (QWidget *parent)
			: QWizardPage (parent)
			{
				setupUi (this);
			}
			
			QStringList SecondStep::GetPaths () const
			{
				QStringList result;
				for (int i = 0; i < FilesWidget_->topLevelItemCount (); ++i)
					result << FilesWidget_->topLevelItem (i)->text (1);
				return result;
			}
			
			void SecondStep::on_AddPath__released ()
			{
				QStringList paths = QFileDialog::getOpenFileNames (this,
						tr ("Select one or more paths to add"),
						XmlSettingsManager::Instance ()->
							property ("LastAddDirectory").toString ());
				if (paths.isEmpty ())
					return;
			
				XmlSettingsManager::Instance ()->setProperty ("LastAddDirectory",
						paths.at (0));
				
				QStringList files = paths;
				for (int i = 0; i < files.size (); ++i)
				{
					QString path = files.at (i);
					QTreeWidgetItem *item = new QTreeWidgetItem (FilesWidget_);
					item->setText (0,
							LeechCraft::Util::Proxy::Instance ()->
							MakePrettySize (QFileInfo (path).size ()));
					item->setText (1, path);
				}
			}
			
			void SecondStep::on_RemoveSelected__released ()
			{
				QList<QTreeWidgetItem*> items = FilesWidget_->selectedItems ();
				qDeleteAll (items);
			}
			
			void SecondStep::on_Clear__released ()
			{
				FilesWidget_->clear ();
			}
			
		};
	};
};

