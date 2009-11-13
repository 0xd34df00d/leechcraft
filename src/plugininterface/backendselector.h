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

#ifndef PLUGININTERFACE_BACKENDSELECTOR_H
#define PLUGININTERFACE_BACKENDSELECTOR_H
#include <QWidget>
#include "config.h"
#include "../xmlsettingsdialog/basesettingsmanager.h"

namespace Ui
{
	class BackendSelector;
};

namespace LeechCraft
{
	namespace Util
	{
		/** A common dialog to select storage backend.
		 *
		 * Currently following backends are supported:
		 * - SQLite
		 * - PostgreSQL
		 *   If driver is not available, PostgreSQL will be grayed out.
		 *
		 * Communication is performed via BaseSettingsManager object
		 * passed to the constructor. The following properties are used
		 * in it:
		 * - StorageType of type QString
		 *   Could be either SQLite or PostgreSQL
		 * - SQLiteVacuum of type bool
		 *   Used if SQLite is chosen to set the VACUUM option.
		 * - SQLiteJournalMode of type string
		 *   Used if SQLite is chosen to set the desired journal mode.
		 * - SQLiteTempStore of type string
		 *   Used if SQLite is chosen to set the temporary storage.
		 * - SQLiteSynchronous of type string
		 *   Used if SQLite is chosen to set the sync mode.
		 * - PostgresHostname
		 *   Used if PostgreSQL is chosen to set server's host name.
		 * - PostgresPort
		 *   Used if PostgreSQL is chosen to set server's port.
		 * - PostgresDBName
		 *   Used if PostgreSQL is chosen to set database name.
		 * - PostgresUsername
		 *   Used if PostgreSQL is chosen to set user name for the
		 *   database.
		 * - PostgresPassword
		 *   Used if PostgreSQL is chosen to set password for the
		 *   database.
		 *
		 * These settings are also queried when constructing the
		 * selector to use them as default ones.
		 */
		class PLUGININTERFACE_API BackendSelector : public QWidget
		{
			Q_OBJECT

			Ui::BackendSelector *Ui_;
			BaseSettingsManager *Manager_;
		public:
			/** Constructs the BackendSelector from the given manager
			 * and parent widget.
			 *
			 * @param[in,out] manager The settings manager to use to
			 * communicate with the outer world.
			 * @param[in] parent The parent widget.
			 */
			BackendSelector (BaseSettingsManager *manager, QWidget *parent = 0);
		private:
			/** Fills the user interface according to the settings in
			 * the manager.
			 */
			void FillUI ();
		public slots:
			/** Fills the settings manager with the settings from the
			 * user interface.
			 */
			void accept ();

			/** Restores the settings in the user interface according to
			 * the manager.
			 */
			void reject ();
		};
	};
};

#endif

