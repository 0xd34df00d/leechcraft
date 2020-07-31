/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "infoserverwrapper.h"

namespace LC
{
namespace DBus
{
	InfoServerWrapper::InfoServerWrapper (IInfo *info)
	: W_ { info }
	{
	}

	void InfoServerWrapper::SetProxy (ICoreProxy_ptr proxy)
	{
		W_->SetProxy (proxy);
	}

	void InfoServerWrapper::Init (ICoreProxy_ptr proxy)
	{
		W_->Init (proxy);
	}

	void InfoServerWrapper::SecondInit ()
	{
		W_->SecondInit ();
	}

	void InfoServerWrapper::Release ()
	{
		W_->Release ();
	}

	QByteArray InfoServerWrapper::GetUniqueID () const
	{
		return W_->GetUniqueID ();
	}

	QString InfoServerWrapper::GetName () const
	{
		return W_->GetName ();
	}

	QString InfoServerWrapper::GetInfo () const
	{
		return W_->GetInfo ();
	}

	QIcon InfoServerWrapper::GetIcon () const
	{
		return W_->GetIcon ();
	}
}
}
