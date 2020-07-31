/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QTimer>
#include "storage.h"
#include "accountsmanager.h"
#include "operationsmanager.h"
#include "currenciesmanager.h"

namespace LC
{
namespace Poleemery
{
	Core::Core ()
	: Storage_ (new Storage)
	, AccsManager_ (new AccountsManager (Storage_))
	, OpsManager_ (new OperationsManager (Storage_))
	, CurrenciesManager_ (new CurrenciesManager ())
	{
		QTimer::singleShot (0,
				this,
				SLOT (postInit ()));
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetCoreProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		CurrenciesManager_->Load ();
	}

	ICoreProxy_ptr Core::GetCoreProxy () const
	{
		return Proxy_;
	}

	Storage_ptr Core::GetStorage () const
	{
		return Storage_;
	}

	AccountsManager* Core::GetAccsManager () const
	{
		return AccsManager_;
	}

	OperationsManager* Core::GetOpsManager () const
	{
		return OpsManager_;
	}

	CurrenciesManager* Core::GetCurrenciesManager () const
	{
		return CurrenciesManager_;
	}

	void Core::postInit ()
	{
		OpsManager_->Load ();
	}
}
}
