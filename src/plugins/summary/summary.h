/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_SUMMARY_SUMMARY_H
#define PLUGINS_SUMMARY_SUMMARY_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/iembedtab.h>
#include <interfaces/imultitabs.h>
#include <interfaces/isummaryrepresentation.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			class Summary : public QObject
						  , public IInfo
						  , public IEmbedTab
						  , public IMultiTabs
						  , public ISummaryRepresentation
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IEmbedTab IMultiTabs ISummaryRepresentation)

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

				QWidget* GetTabContents ();
				QToolBar* GetToolBar () const;

				QModelIndex MapToSource (const QModelIndex&) const;
				QObject* GetTreeViewReemitter () const;
				QTreeView* GetCurrentView () const;
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

