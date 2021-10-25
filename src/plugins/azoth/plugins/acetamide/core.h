/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "localtypes.h"

class QStandardItemModel;

namespace LC::Azoth
{
	class IProxyObject;
}

namespace LC::Azoth::Acetamide
{
	class IrcProtocol;
	class IrcAccount;
	class NickServIdentifyWidget;

	class Core : public QObject
	{
		Q_OBJECT

		std::shared_ptr<IrcProtocol> IrcProtocol_;
		QObject *PluginProxy_ = nullptr;
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

		NickServIdentifyWidget* GetNickServIdentifyWidget () const;

		void AddNickServIdentify (const NickServIdentify&);
		QList<NickServIdentify> GetNickServIdentifyWithMainParams (const QString&,
				const QString&, const QString&) const;
	};
}
