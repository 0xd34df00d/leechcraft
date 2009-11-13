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

#ifndef HANDLERCHOICEDIALOG_H
#define HANDLERCHOICEDIALOG_H
#include <map>
#include <memory>
#include <QDialog>
#include <QButtonGroup>
#include "ui_handlerchoicedialog.h"

class IInfo;
class IDownload;
class IEntityHandler;

namespace LeechCraft
{
	/** Dialog to allow the user to choose the downloader or handler he
	 * wants for a given entity.
	 *
	 * There is a list of downloaders and a list of handlers. If a
	 * downloader is selected, a combobox to select download location
	 * becomes active, where there are previous download locations and
	 * special "Browser" item. This dialog keeps track of the download
	 * locations, first suggesting previous locations for this exact
	 * plugin, followed by all other locations.
	 */
	class HandlerChoiceDialog : public QDialog
	{
		Q_OBJECT

		Ui::HandlerChoiceDialog Ui_;
		std::auto_ptr<QButtonGroup> Buttons_;
		typedef std::map<QString, IDownload*> downloaders_t;
		downloaders_t Downloaders_;
		typedef std::map<QString, IEntityHandler*> handlers_t;
		handlers_t Handlers_;
		mutable QString Suggestion_;
	public:
		/** Constructs the dialog for the given entity.
		 *
		 * @param[in] entity The human-readable text representing the
		 * entity.
		 * @param[in] parent The parent of this dialog.
		 */
		HandlerChoiceDialog (const QString& entity, QWidget *parent = 0);

		/** Sets the suggested file name. That is, the
		 * DownloadEntity::Location field.
		 *
		 * @param[in] suggestion The suggested filename.
		 */
		void SetFilenameSuggestion (const QString& filename);

		/** Adds a downloader to the list of downloaders.
		 *
		 * @param[in] ii The IInfo portion of this downloader.
		 * @param[in] id The IDownload portion of this downloader.
		 * @return True if addition was successful, false otherwise.
		 */
		bool Add (const IInfo *ii, IDownload *id);

		/* Adds a handler to the list of handlers.
		 *
		 * @param[in] ii The IInfo portion of this handler.
		 * @param[in] ieh The IEntityHandler portion of this handler.
		 * @return True if addition was successful, false otherwise.
		 */
		bool Add (const IInfo *ii, IEntityHandler *ieh);

		/** Returns the selected downloader or NULL if no downloader was
		 * selected by user.
		 *
		 * @return The selected downloader or NULL if no downloader was
		 * selected.
		 */
		IDownload* GetDownload ();

		/** Returns the first downloader from the list of downloaders or
		 * NULL if no downloaders were added to the dialog by calling
		 * Add().
		 *
		 * @return The first downloader or NULL if no downloaders were
		 * added.
		 */
		IDownload* GetFirstDownload ();

		/** Returns the selected entity handler or NULL if no entity
		 * handler was selected by user.
		 *
		 * @return The selected entity handler or NULL if no entity
		 * handler was selected.
		 */
		IEntityHandler* GetEntityHandler ();

		/** Returns the first entity handler from the list of entity
		 * handlers or NULL if no entity handlers were added by calling
		 * Add().
		 *
		 * @return The first entity handler or NULL if no entity
		 * handlers were added.
		 */
		IEntityHandler* GetFirstEntityHandler ();

		/** Returns the selected file name. If "Browse" item was
		 * selected, this function automatically requests the save
		 * location from user, records and returns it.
		 *
		 * @return The selected (or entered) filename.
		 */
		QString GetFilename () const;

		/** Returns the total number of choices added.
		 *
		 * @return The total number of choices.
		 */
		int NumChoices () const;
	private:
		/** Returns all save paths for the plugin identified by its
		 * name (IInfo::GetName()).
		 *
		 * @param[in] name The name of the plugin.
		 * @return The previous save paths for this plugin.
		 */
		QStringList GetPluginSavePaths (const QString& name) const;
	private slots:
		/** Fills the combobox with save locations with items in
		 * right order:
		 * - First, the suggested filename if it isn't empty.
		 * - Second, previous save locations for this plugin.
		 * - Third, save locations for all other plugins.
		 */
		void populateLocationsBox ();
	};
};

#endif

