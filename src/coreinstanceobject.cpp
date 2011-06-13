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

#include "coreinstanceobject.h"
#include <QIcon>
#include <QDir>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QStyleFactory>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "pluginmanagerdialog.h"
#include "skinengine.h"
#include "tagsviewer.h"
#include "core.h"

namespace LeechCraft
{
	namespace
	{
		QMap<QString, QString> GetInstalledLanguages ()
		{
			QStringList filenames;

	#ifdef Q_WS_WIN
			filenames << QDir (QCoreApplication::applicationDirPath () + "/translations")
					.entryList (QStringList ("leechcraft_*.qm"));
	#elif defined (Q_WS_MAC)
			filenames << QDir (QCoreApplication::applicationDirPath () + "/../Resources/translations")
					.entryList (QStringList ("leechcraft_*.qm"));
	#elif defined (INSTALL_PREFIX)
			filenames << QDir (INSTALL_PREFIX "/share/leechcraft/translations")
					.entryList (QStringList ("leechcraft_*.qm"));
	#else
			filenames << QDir ("/usr/local/share/leechcraft/translations")
					.entryList (QStringList ("leechcraft_*.qm"));
			filenames << QDir ("/usr/share/leechcraft/translations")
					.entryList (QStringList ("leechcraft_*.qm"));
	#endif

			int length = QString ("leechcraft_").size ();
			QMap<QString, QString> languages;
			Q_FOREACH (QString fname, filenames)
			{
				fname = fname.mid (length);
				fname.chop (3);					// for .qm
				QStringList parts = fname.split ('_', QString::SkipEmptyParts);

				QString language;
				Q_FOREACH (const QString& part, parts)
				{
					if (part.size () != 2)
						continue;
					if (!part.at (0).isLower ())
						continue;

					QLocale locale (part);
					if (locale.language () == QLocale::C)
						continue;

					language = QLocale::languageToString (locale.language ());

					while (part != parts.at (0))
						parts.pop_front ();

					languages [language] = parts.join ("_");
					break;
				}
			}

			return languages;
		}

		QAbstractItemModel* GetInstalledLangsModel ()
		{
			QMap<QString, QString> languages = GetInstalledLanguages ();

			QStandardItemModel *model = new QStandardItemModel ();
			QStandardItem *systemItem = new QStandardItem (QObject::tr ("System"));
			systemItem->setData ("system", Qt::UserRole);
			model->appendRow (systemItem);
			Q_FOREACH (const QString& language, languages.keys ())
			{
				QStandardItem *item = new QStandardItem (language);
				item->setData (languages [language], Qt::UserRole);
				model->appendRow (item);
			}
			return model;
		}
	}

	CoreInstanceObject::CoreInstanceObject (QObject *parent)
	: QObject (parent)
	, XmlSettingsDialog_ (new Util::XmlSettingsDialog ())
	{
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"coresettings.xml");
	}
	
	void CoreInstanceObject::Init (ICoreProxy_ptr proxy)
	{
		XmlSettingsDialog_->SetCustomWidget ("PluginManager", new PluginManagerDialog);
		XmlSettingsDialog_->SetCustomWidget ("TagsViewer", new TagsViewer);
		
		XmlSettingsDialog_->SetDataSource ("Language",
			GetInstalledLangsModel ());
		XmlSettingsDialog_->SetDataSource ("IconSet",
			new QStringListModel (SkinEngine::Instance ().ListIcons ()));
		
		QStringList appQStype = QStyleFactory::keys ();
		appQStype.prepend ("Default");
		XmlSettingsDialog_->SetDataSource ("AppQStyle",
				new QStringListModel (appQStype));
	}
	
	void CoreInstanceObject::SecondInit ()
	{
		QStandardItemModel *newTabsModel = new QStandardItemModel (this);
		QStandardItem *defaultItem = new QStandardItem (tr ("Context-dependent"));
		defaultItem->setData ("contextdependent", Qt::UserRole);
		newTabsModel->appendRow (defaultItem);

		QObjectList multitabs = Core::Instance ()
				.GetPluginManager ()->GetAllCastableRoots<IHaveTabs*> ();
		Q_FOREACH (QObject *object, multitabs)
		{
			IInfo *ii = qobject_cast<IInfo*> (object);
			IHaveTabs *iht = qobject_cast<IHaveTabs*> (object);
			Q_FOREACH (const TabClassInfo& info, iht->GetTabClasses ())
			{
				QStandardItem *item =
						new QStandardItem (ii->GetName () + ": " + info.VisibleName_);
				item->setToolTip (info.Description_);
				item->setIcon (info.Icon_);
				item->setData (ii->GetUniqueID () + '|' + info.TabClass_, Qt::UserRole);
				newTabsModel->appendRow (item);
			}
		}

		qDebug () << Q_FUNC_INFO
				<< "DefaultNewTab"
				<< XmlSettingsManager::Instance ()->property ("DefaultNewTab");

		Core::Instance ().GetCoreInstanceObject ()->
				GetSettingsDialog ()->SetDataSource ("DefaultNewTab", newTabsModel);
	}
	
	void CoreInstanceObject::Release ()
	{
		XmlSettingsDialog_.reset ();
	}

	QByteArray CoreInstanceObject::GetUniqueID () const
	{
		return "org.LeechCraft.CoreInstance";
	}
	
	QString CoreInstanceObject::GetName () const
	{
		return "LeechCraft";
	}
	
	QString CoreInstanceObject::GetInfo () const
	{
		return tr ("LeechCraft Core module.");
	}
	
	QIcon CoreInstanceObject::GetIcon () const
	{
		return QIcon (":/resources/images/leechcraft.svg");
	}
	
	Util::XmlSettingsDialog_ptr CoreInstanceObject::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}
}
