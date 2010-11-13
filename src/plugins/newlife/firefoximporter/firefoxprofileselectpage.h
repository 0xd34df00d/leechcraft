/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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


#ifndef PLUGINS_NEWLIFE_FIREFOXPROFILESELECTPAGE_H
#define PLUGINS_NEWLIFE_FIREFOXPROFILESELECTPAGE_H

#include "ui_firefoxprofileselectpage.h"
#include <boost/shared_ptr.hpp>
#include <QWizardPage>


class QSqlDatabase;
class QSqlQuery;

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace NewLife
		{
			class FirefoxProfileSelectPage : public QWizardPage
			{
				Q_OBJECT
				
					Ui::FirefoxProfileSelectPage Ui_;
					boost::shared_ptr<QSqlDatabase> DB_;
				public:
					FirefoxProfileSelectPage (QWidget* = 0);
					virtual ~FirefoxProfileSelectPage ();
					
					virtual int nextId () const;
					virtual void initializePage ();
					QString GetProfileDirectory (const QString&) const;
					void GetProfileList (const QString&);
					QList<QVariant> GetHistory ();
					QList<QVariant> GetBookmarks ();
					QString GetImportOpmlFile ();
					QSqlQuery GetQuery (const QString&);
					bool IsFirefoxRunning ();
				private slots:
					void checkImportDataAvailable (int);
					void handleAccepted ();
				signals:
					void gotEntity (const LeechCraft::Entity&);
			};
		};
	};
};
#endif // PLUGINS_NEWLIFE_FIREFOXPROFILESELECTPAGE_H
