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
#include <interfaces/core/icoreproxyfwd.h>
#include "ui_artistbrowsertab.h"

class QQuickWidget;

namespace LC
{
namespace LMP
{
	class BioViewManager;
	class SimilarViewManager;

	class ArtistBrowserTab : public QWidget
						   , public ITabWidget
						   , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		const TabClassInfo TC_;
		QObject * const Plugin_;

		QQuickWidget * const View_;

		Ui::ArtistBrowserTab Ui_;

		BioViewManager * const BioMgr_;
		SimilarViewManager * const SimilarMgr_;

		const ICoreProxy_ptr Proxy_;
	public:
		ArtistBrowserTab (const ICoreProxy_ptr&, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;

		void Browse (const QString&);
	private slots:
		void on_ArtistNameEdit__returnPressed ();
	signals:
		void removeTab (QWidget*);

		void tabRecoverDataChanged ();
	};
}
}
