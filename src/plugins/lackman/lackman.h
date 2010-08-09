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

#ifndef PLUGINS_LACKMAN_LACKMAN_H
#define PLUGINS_LACKMAN_LACKMAN_H
#include <QWidget>
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iembedtab.h>
#include <interfaces/ihavesettings.h>
#include "ui_lackman.h"

class QSortFilterProxyModel;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			class TypeFilterProxyModel;

			class Plugin : public QWidget
						 , public IInfo
						 , public IEmbedTab
						 , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEmbedTab IHaveSettings)

				Ui::LackMan Ui_;
				std::auto_ptr<QTranslator> Translator_;
				QSortFilterProxyModel *FilterString_;
				QSortFilterProxyModel *FilterByTags_;
				TypeFilterProxyModel *TypeFilter_;
				Util::XmlSettingsDialog_ptr SettingsDialog_;
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

				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;

				Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
			private slots:
				void handleTagsUpdated ();
				void on_Apply__released ();
				void on_Cancel__released ();
				void on_PackageStatus__currentIndexChanged (int);
			signals:
				void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
				void gotEntity (const LeechCraft::Entity&);
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

