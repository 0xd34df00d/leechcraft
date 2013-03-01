/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/lmp/ilmpplugin.h>
#include "ui_graffititab.h"

class QFileSystemModel;

namespace Media
{
	struct AudioInfo;
}

namespace LeechCraft
{
namespace LMP
{
struct MediaInfo;

namespace Graffiti
{
	class FilesModel;
	class FilesWatcher;

	class GraffitiTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		ICoreProxy_ptr CoreProxy_;
		ILMPProxy_ptr LMPProxy_;

		const TabClassInfo TC_;
		QObject * const Plugin_;

		Ui::GraffitiTab Ui_;

		QFileSystemModel *FSModel_;
		FilesModel *FilesModel_;
		FilesWatcher *FilesWatcher_;

		std::shared_ptr<QToolBar> Toolbar_;
		QAction *Save_;
		QAction *Revert_;
		QAction *RenameFiles_;
		QAction *GetTags_;
		QAction *SplitCue_;

		bool IsChangingCurrent_;
	public:
		GraffitiTab (ICoreProxy_ptr, ILMPProxy_ptr, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		template<typename T, typename F>
		void UpdateData (const T& newData, F getter);
	private slots:
		void on_Artist__textChanged (const QString&);
		void on_Album__textChanged (const QString&);
		void on_Title__textChanged (const QString&);
		void on_Genre__textChanged (const QString&);
		void on_Year__valueChanged (int);

		void save ();
		void revert ();
		void renameFiles ();
		void fetchTags ();
		void splitCue ();

		void handleTagsFetched (const QString&, const Media::AudioInfo&);

		void on_DirectoryTree__activated (const QModelIndex&);
		void currentFileChanged (const QModelIndex&);
		void handleRereadFiles ();

		void handleIterateFinished ();
		void handleScanFinished ();

		void handleCueSplitError (const QString&);
		void handleCueSplitFinished ();
	signals:
		void removeTab (QWidget*);
	};
}
}
}
