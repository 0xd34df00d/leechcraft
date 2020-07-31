/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_proxiesconfigwidget.h"
#include "structures.h"

class QStandardItemModel;

namespace LC
{
namespace XProxy
{
	class ProxiesStorage;
	class ScriptsManager;

	class ProxiesConfigWidget : public QWidget
	{
		Q_OBJECT

		Ui::ProxiesConfigWidget Ui_;

		ProxiesStorage * const Storage_;
		ScriptsManager * const ScriptsMgr_;

		QStandardItemModel * const Model_;
		QList<Proxy> Proxies_;
	public:
		ProxiesConfigWidget (ProxiesStorage*, ScriptsManager*, QWidget* = 0);
	public slots:
		void accept ();
		void reject ();
	private slots:
		void handleItemSelected (const QModelIndex&);

		void on_AddProxyButton__released ();
		void on_UpdateProxyButton__released ();
		void on_RemoveProxyButton__released ();
		void on_MoveUpButton__released ();
		void on_MoveDownButton__released ();

		void on_EditUrlsButton__released ();
		void on_EditListsButton__released ();
	};
}
}
