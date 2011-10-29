/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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

#ifndef PLUGINS_LAURE_PLAYLISTADDMENU_H
#define PLUGINS_LAURE_PLAYLISTADDMENU_H
#include <QMenu>
#include <QFileInfoList>
#include <boost/shared_ptr.hpp>
#include <magic.h>

namespace LeechCraft
{
namespace Laure
{
	class PlayListView;
	class PlayListAddMenu : public QMenu
	{
		Q_OBJECT
#ifdef HAVE_MAGIC
		boost::shared_ptr<magic_set> Magic_;
#else
		QStringList Formats_;
#endif
	public:
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
		void addItem (const QString&);
	};
}
}
#endif // PLUGINS_LAURE_PLAYLISTADDMENU_H
