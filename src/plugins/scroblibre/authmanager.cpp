/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authmanager.h"
#include <algorithm>
#include <QTimer>
#include "singleaccauth.h"

namespace LC
{
namespace Scroblibre
{
	AuthManager::AuthManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	, SubmitTimer_ { new QTimer (this) }
	{
		SubmitTimer_->setSingleShot (true);
		connect (SubmitTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (submit ()));
	}

	void AuthManager::HandleAudio (const Media::AudioInfo& info)
	{
		HandleStopped ();

		if (info.Length_ && info.Length_ < 30)
			return;

		if (info.Length_)
			SubmitTimer_->start (std::min (240, info.Length_ / 2) * 1000);
		LastSubmit_ = info;

		for (const auto& subhash : AccAuths_)
			for (const auto auth : subhash)
				auth->SetNP (LastSubmit_);
	}

	void AuthManager::HandleStopped ()
	{
		SubmitTimer_->stop ();
		if (!LastSubmit_.IsValid ())
			return;

		const int secsTo = LastSubmit_.TS_.secsTo (QDateTime::currentDateTime ());
		if (!LastSubmit_.Info_.Length_ && secsTo > 30)
		{
			LastSubmit_.Info_.Length_ = secsTo;
			submit ();
		}
	}

	void AuthManager::handleAccountAdded (const QUrl& service, const QString& login)
	{
		AccAuths_ [service] [login] = new SingleAccAuth (service, login, Proxy_, this);
	}

	void AuthManager::handleAccountRemoved (const QUrl& service, const QString& login)
	{
		delete AccAuths_ [service].take (login);
	}

	void AuthManager::submit ()
	{
		for (const auto& subhash : AccAuths_)
			for (const auto auth : subhash)
				auth->Submit (LastSubmit_);
	}
}
}
