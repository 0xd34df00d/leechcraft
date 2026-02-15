/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <util/models/mergemodel.h>
#include <interfaces/iinfo.h>
#include <interfaces/ihaverecoverabletabs.h>

class QSortFilterProxyModel;
class QTreeView;
class QToolBar;

namespace LC
{
namespace Summary
{
	class SummaryTagsFilter;
	class SummaryWidget;
	class TreeViewReemitter;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		QPointer<SummaryWidget> Current_;

		Core ();
	public:
		static Core& Instance ();
		void Release ();
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		void SecondInit ();

		void RecoverTabs (const QList<TabRecoverInfo>&);
	public slots:
		void handleNewTabRequested ();
	private:
		template<typename F>
		SummaryWidget* CreateSummaryWidget (F&& f);
	private slots:
		void handleCurrentTabChanged (int);
		void handleWindow (int);
	signals:
		void currentViewChanged (QTreeView*);
	};
}
}
