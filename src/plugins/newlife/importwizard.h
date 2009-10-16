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

#ifndef PLUGINS_NEWLIFE_IMPORTWIZARD_H
#define PLUGINS_NEWLIFE_IMPORTWIZARD_H
#include <QWizard>
#include "ui_importwizard.h"

namespace LeechCraft
{
	struct DownloadEntity;

	namespace Plugins
	{
		namespace NewLife
		{
			class AbstractImporter;
			class FirstPage;

			class ImportWizard : public QWizard
			{
				Q_OBJECT

				friend class FirstPage;

				Ui::ImportWizard Ui_;
				QList<AbstractImporter*> Importers_;
			public:
				ImportWizard (QWidget* = 0);

				QString GetSelectedName () const;
			private slots:
				void handleAccepted ();
				void handleRejected ();
			private:
				void SetupImporters ();
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

