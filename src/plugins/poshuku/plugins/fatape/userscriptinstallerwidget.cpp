/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "userscriptinstallerwidget.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkRequest>
#include "fatape.h"
#include "userscript.h"



namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{


	UserScriptInstallerDialog::UserScriptInstallerDialog (Plugin *plugin,  
			QNetworkAccessManager *networkManager, const QUrl& scriptUrl, QWidget *parent) 
	: QDialog (parent)
	, Plugin_ (plugin)
	{
		QDir temp (QDesktopServices::storageLocation (QDesktopServices::TempLocation));
		QFileInfo userScript (temp, QFileInfo (scriptUrl.path ()).fileName ());
		

		Ui_.setupUi (this);
		TempScriptPath_  = userScript.absoluteFilePath ();

		QNetworkRequest scriptRequest;
		scriptRequest.setUrl (scriptUrl);
		connect (networkManager, 
				SIGNAL (finished (QNetworkReply*)), 
				this, 
				SLOT (ScriptFetchFinished (QNetworkReply*)));
		networkManager->get (scriptRequest);
		Ui_.ScriptInfo_->setHtml (QString ("<i>%1</i>").arg (tr ("Fetching script...")));
		connect (Ui_.Install_,
				SIGNAL (released  ()),
				this,
				SLOT (On_Install__released ()));
		connect (Ui_.ShowSource_,
				SIGNAL(released ()),
				this,
				SLOT(On_ShowSource__released ()));
		connect (Ui_.Cancel_,
				SIGNAL(released ()),
				this,
				SLOT(On_Cancel__released ()));

	}

	void UserScriptInstallerDialog::ScriptFetchFinished (QNetworkReply *reply)
	{
		QFile tempScript (TempScriptPath_);

		if (tempScript.open (QFile::ReadWrite))
		{
			tempScript.write (reply->readAll ());
			tempScript.close ();
		}

		UserScript script (TempScriptPath_);
		QString scriptDesc = QString ("<b>%1</b><br />%2<hr />%3<br /><i>%4</i>")
				.arg (script.Name ())
				.arg (script.Description ())
				.arg (tr ("runs on:"))
				.arg (script.Include ().join ("<br />"));

		if (!script.Exclude ().isEmpty ())
		{
			scriptDesc.append (QString ("<br /><br />%1<br /><i>%2</i>")
					.arg (tr ("does not run on:"))
					.arg (script.Exclude ().join ("<br />")));
		}
		Ui_.ScriptInfo_->setHtml (scriptDesc);
	}

	void UserScriptInstallerDialog::On_Install__released ()
	{
		done (Install);
	}

	void UserScriptInstallerDialog::On_ShowSource__released ()
	{
		done (ShowSource);
	}

	void UserScriptInstallerDialog::On_Cancel__released ()
	{
		done (Cancel);
	}

	QString UserScriptInstallerDialog::TempScriptPath () const
	{
		return TempScriptPath_;
	}
}
}
}

