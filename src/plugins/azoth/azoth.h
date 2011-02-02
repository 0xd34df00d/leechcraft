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

#ifndef PLUGINS_AZOTH_AZOTH_H
#define PLUGINS_AZOTH_AZOTH_H
#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ipluginready.h>
#include <interfaces/imultitabs.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
namespace Azoth
{
	class MainWidget;

	class Plugin : public QObject
					, public IInfo
					, public IPluginReady
					, public IMultiTabs
					, public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPluginReady IMultiTabs IHaveSettings)

		MainWidget *MW_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		std::auto_ptr<QTranslator> Translator_;
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

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	public slots:
		void newTabRequested ();
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);

		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
}
}

#endif

