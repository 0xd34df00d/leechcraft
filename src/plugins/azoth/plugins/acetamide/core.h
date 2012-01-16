/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "localtypes.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcProtocol;
	class IrcAccount;
	class NickServIdentifyWidget;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		std::shared_ptr<IrcProtocol> IrcProtocol_;
		QObject *PluginProxy_;
		QStandardItemModel* Model_;
		NickServIdentifyWidget* NickServIdentifyWidget_;
		QList<NickServIdentify> NickServIdentifyList_;

		Core ();
	public:
		static Core& Instance ();

		void Init ();
		void SecondInit ();
		void Release ();
		QList<QObject*> GetProtocols () const;

		void SetPluginProxy (QObject*);
		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;
		void SendEntity (const Entity&);

		NickServIdentifyWidget* GetNickServIdentifyWidget () const;
		QStandardItemModel* GetNickServIdentifyModel () const;

		void AddNickServIdentify (const NickServIdentify&);
		QList<NickServIdentify> GetAllNickServIdentify () const;
		QList<NickServIdentify> GetNickServIdentifyWithNick (const QString&) const;
		QList<NickServIdentify> GetNickServIdentifyWithNickServ (const QString&) const;
		QList<NickServIdentify> GetNickServIdentifyWithServ (const QString&) const;
		QList<NickServIdentify> GetNickServIdentifyWithMainParams (const QString&,
				const QString&, const QString&) const;
	private slots:
		void handleItemsAdded (const QList<QObject*>&);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
