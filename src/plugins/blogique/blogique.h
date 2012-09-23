/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ipluginready.h>
#include <interfaces/iactionsexporter.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LeechCraft
{
namespace Blogique
{
	class Plugin : public QObject
				, public IInfo
				, public IHaveTabs
				, public IHaveSettings
				, public IPluginReady
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveSettings IPluginReady)

		TabClasses_t TabClasses_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray& tabClass);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject* plugin);
	private:
		void CreateTab ();

	signals:
		void addNewTab (const QString& name, QWidget *tabContents);
		void removeTab (QWidget *tabContents);
		void changeTabName (QWidget *tabContents, const QString& name);
		void changeTabIcon (QWidget *tabContents, const QIcon& icon);
		void statusBarChanged (QWidget *tabContents, const QString& text);
		void raiseTab (QWidget *tabContents);

		void gotEntity (const LeechCraft::Entity& e);
		void delegateEntity (const LeechCraft::Entity& e, int *id, QObject **obj);

		void gotActions (QList<QAction*> actions, ActionsEmbedPlace area);
	};
}
}
