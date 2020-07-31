/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <util/sll/eithercont.h>
#include "structures.h"

namespace LC
{
namespace XProxy
{
	class UrlListScript;
	class ScriptsManager;

	class ProxiesStorage : public QObject
	{
		const ScriptsManager * const ScriptsMgr_;

		QList<QPair<Proxy, QList<ReqTarget>>> Proxies_;
		QMap<Proxy, QList<UrlListScript*>> Scripts_;
	public:
		ProxiesStorage (const ScriptsManager*, QObject* = nullptr);

		QList<Proxy> GetKnownProxies () const;

		QList<Proxy> FindMatching (const QString& reqHost, int reqPort,
				const QString& proto = QString ()) const;

		void AddProxy (const Proxy&);
		void UpdateProxy (const Proxy&, const Proxy&);
		void RemoveProxy (const Proxy&);

		QList<ReqTarget> GetTargets (const Proxy&) const;
		void SetTargets (const Proxy&, const QList<ReqTarget>&);

		QList<UrlListScript*> GetScripts (const Proxy&) const;
		void SetScripts (const Proxy&, const QList<UrlListScript*>&);

		void Swap (int, int);

		void LoadSettings ();
		void SaveSettings () const;
	private:
		void EraseFromProxiesList (const Proxy&);

		template<typename R = void>
		R DoOnProxiesList (const Proxy&,
				const Util::EitherCont<R (), R (decltype (Proxies_.begin ()))>&);

		template<typename R = void>
		R DoOnProxiesList (const Proxy&,
				const Util::EitherCont<R (), R (decltype (Proxies_.constBegin ()))>&) const;
	};
}
}
