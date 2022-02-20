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

class QFileInfo;
class QFileSystemModel;

namespace Media
{
	struct AudioInfo;
}

namespace LC::LMP
{
	struct MediaInfo;
}

namespace LC::LMP::Graffiti
{
	class FilesModel;
	class FilesWatcher;
	class CueSplitter;

	class GraffitiTab : public QWidget
					  , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

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
	public:
		GraffitiTab (ILMPProxy_ptr, TabClassInfo, QObject*);

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;

		void SetPath (const QString& dir, const QString& filename = {});
	private:
		template<typename T, typename F>
		void UpdateData (const T& newData, F getter);

		void SetupEdits ();
		void SetupViews ();
		void SetupToolbar ();

		void RestorePathHistory ();
		void AddToPathHistory (const QString&);

		void Save ();
		void Revert ();
		void RenameFiles ();
		void FetchTags ();
		void SplitCue ();

		void PopulateFields (const QModelIndex&);
		void RereadFiles ();

		void HandleDirIterateResults (const QList<QFileInfo>&, const QString&);
	signals:
		void removeTab () override;

		void tagsFetchProgress (int, int, QObject*);
		void cueSplitStarted (CueSplitter*);
	};
}
