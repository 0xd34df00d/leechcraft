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

#include "startupfirstpage.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			StartupFirstPage::StartupFirstPage (QWidget *parent)
			: QWizardPage (parent)
			{
				Ui_.setupUi (this);

				setTitle (tr ("BitTorrent"));
				setSubTitle (tr ("Set basic options"));
			}

			void StartupFirstPage::initializePage ()
			{
				connect (wizard (),
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));
			}

			void StartupFirstPage::handleAccepted ()
			{
				QList<QVariant> ports;
				ports << Ui_.LowerPort_->value ()
					<< Ui_.UpperPort_->value ();
				XmlSettingsManager::Instance ()->setProperty ("TCPPortRange", ports);

				XmlSettingsManager::Instance ()->setProperty ("MaxUploads",
						Ui_.UploadConnections_->value ());
				XmlSettingsManager::Instance ()->setProperty ("MaxConnections",
						Ui_.TotalConnections_->value ());

				int sset = Ui_.SettingsSet_->currentIndex ();
				Core::Instance ()->SetPreset (static_cast<Core::SettingsPreset> (sset));
			}
		};
	};
};

