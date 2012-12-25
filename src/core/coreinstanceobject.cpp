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

#include "coreinstanceobject.h"
#include <QIcon>
#include <QDir>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QStyleFactory>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "pluginmanagerdialog.h"
#include "iconthemeengine.h"
#include "colorthemeengine.h"
#include "tagsviewer.h"
#include "core.h"
#include "settingstab.h"
#include "coreplugin2manager.h"
#include "acceptlangwidget.h"
#include "shortcutmanager.h"

namespace LeechCraft
{
	namespace
	{
		QMap<QString, QString> GetInstalledLanguages ()
		{
			QStringList filenames;

	#ifdef Q_OS_WIN32
			filenames << QDir (QCoreApplication::applicationDirPath () + "/translations")
					.entryList (QStringList ("leechcraft_*.qm"));
	#elif defined (Q_OS_MAC)
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
	, SettingsTab_ (new SettingsTab)
	, CorePlugin2Manager_ (new CorePlugin2Manager)
	, ShortcutManager_ (new ShortcutManager)
	{
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"coresettings.xml");
		connect (XmlSettingsDialog_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handleSettingsButton (QString)));

		connect (SettingsTab_,
				SIGNAL (remove (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
	}

	void CoreInstanceObject::Init (ICoreProxy_ptr)
	{
		Classes_ << SettingsTab_->GetTabClassInfo ();

		XmlSettingsDialog_->SetCustomWidget ("PluginManager", new PluginManagerDialog);
		XmlSettingsDialog_->SetCustomWidget ("TagsViewer", new TagsViewer);

		XmlSettingsDialog_->SetDataSource ("Language",
			GetInstalledLangsModel ());

		XmlSettingsDialog_->SetDataSource ("IconSet",
			new QStringListModel (IconThemeEngine::Instance ().ListIcons ()));
		XmlSettingsManager::Instance ()->RegisterObject ("IconSet", this, "updateIconSet");
		updateIconSet ();

		XmlSettingsDialog_->SetDataSource ("ColorTheme",
			new QStringListModel (ColorThemeEngine::Instance ().ListThemes ()));
		XmlSettingsManager::Instance ()->RegisterObject ("ColorTheme", this, "updateColorTheme");
		updateColorTheme ();

		QStringList appQStype = QStyleFactory::keys ();
		appQStype.prepend ("Default");
		XmlSettingsDialog_->SetDataSource ("AppQStyle", new QStringListModel (appQStype));

		XmlSettingsDialog_->SetCustomWidget ("ShortcutManager", ShortcutManager_);
		XmlSettingsDialog_->SetCustomWidget ("AcceptLanguages", new AcceptLangWidget);
	}

	void CoreInstanceObject::SecondInit ()
	{
		BuildNewTabModel ();
		SettingsTab_->Initialize ();

#ifdef STRICT_LICENSING
		QTimer::singleShot (10000,
				this,
				SLOT (notifyLicensing ()));
#endif
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

	TabClasses_t CoreInstanceObject::GetTabClasses () const
	{
		return Classes_;
	}

	void CoreInstanceObject::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "org.LeechCraft.SettingsPane")
		{
			emit addNewTab (tr ("Settings"), SettingsTab_);
			emit raiseTab (SettingsTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	QSet<QByteArray> CoreInstanceObject::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	void CoreInstanceObject::AddPlugin (QObject *plugin)
	{
		CorePlugin2Manager_->AddPlugin (plugin);
	}

	CorePlugin2Manager* CoreInstanceObject::GetCorePluginManager () const
	{
		return CorePlugin2Manager_;
	}

	SettingsTab* CoreInstanceObject::GetSettingsTab () const
	{
		return SettingsTab_;
	}

	IShortcutProxy* CoreInstanceObject::GetShortcutProxy () const
	{
		return ShortcutManager_;
	}

	ShortcutManager* CoreInstanceObject::GetShortcutManager () const
	{
		return ShortcutManager_;
	}

	void CoreInstanceObject::BuildNewTabModel ()
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

	void CoreInstanceObject::handleSettingsButton (const QString& name)
	{
		auto pm = Core::Instance ().GetPluginManager ();
		if (name == "EnableAllPlugins")
			pm->SetAllPlugins (Qt::Checked);
		else if (name == "DisableAllPlugins")
			pm->SetAllPlugins (Qt::Unchecked);
	}

	void CoreInstanceObject::updateIconSet ()
	{
		IconThemeEngine::Instance ().UpdateIconSet (findChildren<QAction*> ());
		IconThemeEngine::Instance ().UpdateIconSet (findChildren<QTabWidget*> ());
	}

	void CoreInstanceObject::updateColorTheme ()
	{
		const auto& theme = XmlSettingsManager::Instance ()->
				property ("ColorTheme").toString ();
		ColorThemeEngine::Instance ().SetTheme (theme);
	}

#ifdef STRICT_LICENSING
	void CoreInstanceObject::notifyLicensing ()
	{
		if (XmlSettingsManager::Instance ()->
				Property ("NotifiedLicensing", false).toBool ())
			return;

		const QString& str = tr ("Due to licensing issues, some artwork "
				"may have been removed from this package. Consider "
				"using the LackMan plugin to install that artwork.");
		emit gotEntity (Util::MakeNotification ("LeechCraft", str, PWarning_));
		XmlSettingsManager::Instance ()->setProperty ("NotifiedLicensing", true);
	}
#endif
}
