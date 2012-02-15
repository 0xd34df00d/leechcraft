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

#include "keywords.h"
#include <QApplication>
#include <QKeyEvent>
#include <QDebug>
#include <QLineEdit>
#include <QUrl>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h> 
#include "keywordsmanagerwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace Keywords
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;
		Model_.reset (new QStandardItemModel);
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Keyword")) 
			<< tr ("Url"));

		QSettings keywords (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_Keywords");

		Q_FOREACH (const QString& keyword, keywords.allKeys ()) 
		{
			const QString& url = keywords.value (keyword).toString ();
			QStandardItem *keywordItem = new QStandardItem (keyword);
			QStandardItem *urlItem = new QStandardItem (url);
			QList<QStandardItem*> items; 

			items << keywordItem << urlItem;
			Model_->appendRow (items);
			UpdateKeywords (keyword, url);
		}

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			"poshukukeywordssettings.xml");
		SettingsDialog_->SetCustomWidget ("KeywordsManagerWidget",
			new KeywordsManagerWidget (Model_.get (), this)); 
	}
	
	void Plugin::SecondInit ()
	{
	}
	
	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.Keywords";
	}
	
	QString Plugin::GetName () const
	{
		return "Poshuku Keywords";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("URL keywords support for the Poshuku browser.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/poshuku/plugins/keywords/resources/images/keywords.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	void Plugin::UpdateKeywords (const QString& keyword, const QString& url)
	{
		Keywords2Urls_ [keyword] = url;
	}

	void Plugin::RemoveKeyword (const QString& keyword)
	{
		Keywords2Urls_.remove (keyword);
	}

	void Plugin::hookURLEditReturnPressed (LeechCraft::IHookProxy_ptr proxy, 
			QObject *browserWidget)
	{
		QLineEdit *urlEdit;

		QMetaObject::invokeMethod (browserWidget,
			"getAddressBar",
			Q_RETURN_ARG (QLineEdit*, urlEdit));

		if (!urlEdit) 
		{
			qWarning () << Q_FUNC_INFO
					<< "unable get url edit"
					<< "from"
					<< browserWidget;
			return;
		}

		if (urlEdit->text ().isEmpty ())
			return;

		QStringList keywords = urlEdit->text ().split (" ", QString::SkipEmptyParts);

		if (keywords.isEmpty())
			return;

		QString redirect = Keywords2Urls_.value (keywords.takeFirst ());

		if (redirect.isEmpty ())
			return;

		while (!keywords.isEmpty ())
			redirect = redirect.arg (keywords.takeFirst ());
			
		urlEdit->setText (redirect);
		QMetaObject::invokeMethod (urlEdit, "returnPressed", Qt::QueuedConnection);
		proxy->CancelDefault ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_keywords, LeechCraft::Poshuku::Keywords::Plugin);
