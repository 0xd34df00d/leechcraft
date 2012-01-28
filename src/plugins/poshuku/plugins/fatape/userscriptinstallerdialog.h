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

#ifndef PLUGINS_POSHUKU_PLUGINS_FATAPE_USERSCRIPTINSTALLERWIDGET_H
#define PLUGINS_POSHUKU_PLUGINS_FATAPE_USERSCRIPTINSTALLERWIDGET_H
#include "ui_userscriptinstallerdialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

namespace LeechCraft
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
