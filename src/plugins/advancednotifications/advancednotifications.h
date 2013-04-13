/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/iquarkcomponentprovider.h>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class GeneralHandler;

	class Plugin : public QObject
				 , public IInfo
				 , public IEntityHandler
				 , public IHaveSettings
				 , public IActionsExporter
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IEntityHandler IHaveSettings IActionsExporter IQuarkComponentProvider)

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		std::shared_ptr<GeneralHandler> GeneralHandler_;

		QuarkComponent_ptr Component_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		EntityTestHandleResult CouldHandle (const Entity&) const;
		void Handle (Entity);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QList<QAction*> GetActions (ActionsEmbedPlace) const;

		QuarkComponents_t GetComponents () const;
	signals:
		void gotEntity (const LeechCraft::Entity&);

		void gotActions (QList<QAction*>, LeechCraft::ActionsEmbedPlace);
	};
}
}
