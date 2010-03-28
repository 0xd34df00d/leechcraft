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
#include "startupsecondpage.h"
#include <plugininterface/backendselector.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			StartupSecondPage::StartupSecondPage (QWidget *parent)
			: QWizardPage (parent)
			, Selector_ (new Util::BackendSelector (XmlSettingsManager::Instance ()))
			{
				Ui_.setupUi (this);
				QHBoxLayout *lay = new QHBoxLayout ();
				lay->addWidget (Selector_);
				Ui_.SelectorContainer_->setLayout (lay);

				setTitle ("Aggregator");
				setSubTitle (tr ("Set storage options"));
			}

			void StartupSecondPage::initializePage ()
			{
				connect (wizard (),
						SIGNAL (accepted ()),
						this,
						SLOT (handleAccepted ()));
				connect (wizard (),
						SIGNAL (accepted ()),
						Selector_,
						SLOT (accept ()));
			}

			void StartupSecondPage::handleAccepted ()
			{
				wizard ()->setProperty ("NeedsRestart", true);
			}
		};
	};
};
