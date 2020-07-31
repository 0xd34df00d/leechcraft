/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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

namespace LC
{
namespace LMP
{
struct MediaInfo;

namespace Graffiti
{
	class FilesModel;
	class FilesWatcher;
	class CueSplitter;

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

		void SetPath (const QString& dir, const QString& filename = {});
	private:
		template<typename T, typename F>
		void UpdateData (const T& newData, F getter);

		void SetupEdits ();
		void SetupViews ();
		void SetupToolbar ();

		void RestorePathHistory ();
		void AddToPathHistory (const QString&);
	private slots:
		void on_Artist__textChanged ();
		void on_Album__textChanged ();
		void on_Title__textChanged ();
		void on_Genre__textChanged ();
		void on_Year__valueChanged ();
		void on_TrackNumber__valueChanged ();

		void on_TrackNumberAutoFill__released ();

		void save ();
		void revert ();
		void renameFiles ();
		void fetchTags ();
		void splitCue ();

		void handleTagsFetched (const QString&);

		void on_DirectoryTree__activated (const QModelIndex&);
		void handlePathLine ();

		void currentFileChanged (const QModelIndex&);
		void handleRereadFiles ();

		void handleIterateFinished ();
		void handleIterateCanceled ();
		void handleScanFinished ();

		void handleCueSplitError (const QString&);
		void handleCueSplitFinished ();
	signals:
		void removeTab (QWidget*);

		void tagsFetchProgress (int, int, QObject*);
		void cueSplitStarted (CueSplitter*);
	};
}
}
}
