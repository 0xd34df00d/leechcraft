/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once
#include <memory>
#include <QMenu>
#include <QFileInfoList>

#ifdef HAVE_MAGIC
	#include <magic.h>
#endif

namespace LeechCraft
{
namespace Laure
{
	class PlayListView;

	/** @brief Provides menu for choosing add item options.
	 *
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayListAddMenu : public QMenu
	{
		Q_OBJECT
#ifdef HAVE_MAGIC
		std::shared_ptr<magic_set> Magic_;
#else
		QStringList Formats_;
#endif
	public:
		/** @brief Constructs a new PlayListAddMenu class
		 * with the given parent.
		 */
		PlayListAddMenu (QWidget* = 0);
	private slots:
		void handleAddUrl ();
		void handleAddFolder ();
		void handleAddFiles ();
		void handleImportPlayList ();
		QFileInfoList StoragedFiles (const QString&);
	private:
		bool IsFileSupported (const QFileInfo&) const;
		void LoadM3U (const QString&);
	signals:
		/** @brief Is emitted when the media file is chosen.
		 *
		 * @param[out] location Media file location.
		 */
		void addItem (const QString& location);
	};
}
}
