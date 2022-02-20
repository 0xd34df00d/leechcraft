/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_artistbrowsertab.h"

class QQuickWidget;

namespace LC::LMP
{
	class BioViewManager;
	class SimilarViewManager;

	class ArtistBrowserTab : public QWidget
						   , public ITabWidget
						   , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		QQuickWidget * const View_;

		Ui::ArtistBrowserTab Ui_;

		BioViewManager * const BioMgr_;
		SimilarViewManager * const SimilarMgr_;
	public:
		explicit ArtistBrowserTab ();

		static const TabClassInfo& GetStaticTabClass ();

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;

		QByteArray GetTabRecoverData () const override;
		QIcon GetTabRecoverIcon () const override;
		QString GetTabRecoverName () const override;

		void Browse (const QString&);
	private:
		void DoQueries (const QString&);
	signals:
		void removeTab () override;

		void tabRecoverDataChanged () override;
	};
}
