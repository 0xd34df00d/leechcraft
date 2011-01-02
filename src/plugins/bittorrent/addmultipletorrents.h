/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_BITTORRENT_ADDMULTIPLETORRENTS_H
#define PLUGINS_BITTORRENT_ADDMULTIPLETORRENTS_H
#include "ui_addmultipletorrents.h"
#include "core.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsLineEdit;
	};

	namespace Plugins
	{
		namespace BitTorrent
		{

			class AddMultipleTorrents : public QDialog, private Ui::AddMultipleTorrents
			{
				Q_OBJECT
			public:
				AddMultipleTorrents (QWidget *parent = 0);
				QString GetOpenDirectory () const;
				QString GetSaveDirectory () const;
				Core::AddType GetAddType () const;
				Util::TagsLineEdit* GetEdit ();
				/** Returns the list of tags after the dialog is executed.
				 *
				 * @return List if IDs of tags.
				 */
				QStringList GetTags () const;
			private slots:
				void on_BrowseOpen__released ();
				void on_BrowseSave__released ();
			};
		};
	};
};

#endif

