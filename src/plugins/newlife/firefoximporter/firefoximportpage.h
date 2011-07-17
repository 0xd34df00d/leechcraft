/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_NEWLIFE_FIREFOXIMPORTPAGE_H
#define PLUGINS_NEWLIFE_FIREFOXIMPORTPAGE_H
#include <QWizardPage>
#include "ui_feedssettingsimportpage.h"

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace NewLife
		{
			class FirefoxImportPage : public QWizardPage
			{
				Q_OBJECT

				Ui::FeedsSettingsImportPage Ui_;				
			public:
				FirefoxImportPage (QWidget* = 0);
				~FirefoxImportPage ();

				bool CheckValidity (const QString&) const;
				virtual bool isComplete () const;
				virtual void initializePage ();
			private slots:
				void on_Browse__released ();
				void on_FileLocation__textEdited (const QString&);
				void handleAccepted (int);
			};
		};
	};
};

#endif

