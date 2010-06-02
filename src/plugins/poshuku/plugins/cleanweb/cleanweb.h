/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CLEANWEB_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CLEANWEB_H
#include <memory>
#include <QObject>
#include <QMap>
#include <QTranslator>
#include <QWebPage>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/iwebplugin.h>
#include <interfaces/poshukutypes.h>

class QWebView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace CleanWeb
				{
					class CleanWeb : public QObject
								   , public IInfo
								   , public IHaveSettings
								   , public IEntityHandler
								   , public IStartupWizard
								   , public IPlugin2
					{
						Q_OBJECT
						Q_INTERFACES (IInfo IHaveSettings IEntityHandler IStartupWizard IPlugin2)

						boost::shared_ptr<Util::XmlSettingsDialog> SettingsDialog_;
						std::auto_ptr<QTranslator> Translator_;
					public:
						void Init (ICoreProxy_ptr);
						void SecondInit ();
						void Release ();
						QString GetName () const;
						QString GetInfo () const;
						QIcon GetIcon () const;
						QStringList Provides () const;
						QStringList Needs () const;
						QStringList Uses () const;
						void SetProvider (QObject*, const QString&);

						boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;

						bool CouldHandle (const Entity&) const;
						void Handle (Entity);

						QList<QWizardPage*> GetWizardPages () const;

						QSet<QByteArray> GetPluginClasses () const;
					public slots:
						void hookExtension (LeechCraft::IHookProxy_ptr,
								QWebPage*,
								QWebPage::Extension,
								const QWebPage::ExtensionOption*,
								QWebPage::ExtensionReturn*,
								bool*);
						void hookWebPluginFactoryReload (LeechCraft::IHookProxy_ptr,
								QList<IWebPlugin*>&);
						void hookWebViewContextMenu (LeechCraft::IHookProxy_ptr,
								QWebView*, QContextMenuEvent*,
								const QWebHitTestResult&, QMenu*,
								WebViewCtxMenuStage);
					signals:
						void delegateEntity (const LeechCraft::Entity&,
								int*, QObject**);
						void gotEntity (const LeechCraft::Entity&);
					};
				};
			};
		};
	};
};

#endif

