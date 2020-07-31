/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Poleemery
{
	class Storage;
	typedef std::shared_ptr<Storage> Storage_ptr;

	class AccountsManager;
	class OperationsManager;
	class CurrenciesManager;

	class Core : public QObject
	{
		Q_OBJECT

		Storage_ptr Storage_;
		AccountsManager *AccsManager_;
		OperationsManager *OpsManager_;
		CurrenciesManager *CurrenciesManager_;

		ICoreProxy_ptr Proxy_;

		Core ();

		Core (const Core&) = delete;
		Core (Core&&) = delete;

		Core& operator= (const Core&) = delete;
		Core& operator= (Core&&) = delete;
	public:
		static Core& Instance ();

		void SetCoreProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetCoreProxy () const;

		Storage_ptr GetStorage () const;

		AccountsManager* GetAccsManager () const;
		OperationsManager* GetOpsManager () const;
		CurrenciesManager* GetCurrenciesManager () const;
	private slots:
		void postInit ();
	};
}
}
