/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_FATAPE_USERSCRIPTINSTALLERWIDGET_H
#define PLUGINS_POSHUKU_PLUGINS_FATAPE_USERSCRIPTINSTALLERWIDGET_H
#include "ui_userscriptinstallerdialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	class Plugin;
	class UserScriptInstallerDialog : public QDialog
    {
		Q_OBJECT

		Plugin *Plugin_;
		QString TempScriptPath_;
		Ui::UserScriptInstallerDialog Ui_;
	public:
		enum Result
		{
			Install,
			ShowSource,
			Cancel
		};

		UserScriptInstallerDialog (Plugin *plugin, QNetworkAccessManager *networkManager,
				const QUrl& scriptUrl, QWidget *parent = 0);
		QString TempScriptPath () const;
	private slots:
		void scriptFetchFinished ();
		void on_Install__released ();
		void on_ShowSource__released ();
		void on_Cancel__released ();
    };
}
}
}

#endif
