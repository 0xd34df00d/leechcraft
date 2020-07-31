/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "localtypes.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
class IProxyObject;

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
		QObject *PluginProxy_ = nullptr;
		QStandardItemModel* Model_;
		NickServIdentifyWidget* NickServIdentifyWidget_;
		QList<NickServIdentify> NickServIdentifyList_;

		Core ();
	public:
		static Core& Instance ();

		void SecondInit ();
		void Release ();
		QList<QObject*> GetProtocols () const;

		void SetPluginProxy (QObject*);
		IProxyObject* GetPluginProxy () const;
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
		void gotEntity (const LC::Entity&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CORE_H
