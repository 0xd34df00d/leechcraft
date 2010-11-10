/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_TABPP_TABPP_H
#define PLUGINS_TABPP_TABPP_H
#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/imultitabs.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

class QTranslator;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			class TabPPWidget;

			class Plugin : public QObject
						 , public IInfo
						 , public IActionsExporter
						 , public IHaveSettings
						 , public IHaveShortcuts
						 , public IMultiTabs
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IActionsExporter IHaveSettings IHaveShortcuts IMultiTabs)

				TabPPWidget *Dock_;
				boost::shared_ptr<Util::XmlSettingsDialog> XmlSettingsDialog_;

				std::auto_ptr<QTranslator> Translator_;
				enum ActionsEnum
				{
					AEActivator
				};
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;
				QStringList Needs () const;
				QStringList Uses () const;
				void SetProvider (QObject*, const QString&);

				QList<QAction*> GetActions (ActionsEmbedPlace) const;

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;

				void SetShortcut (const QString&, const QKeySequences_t&);
				QMap<QString, ActionInfo> GetActionInfo () const;
			public slots:
				void newTabRequested ();
			signals:
				void bringToFront ();
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
			};
		};
	};
};

#endif

