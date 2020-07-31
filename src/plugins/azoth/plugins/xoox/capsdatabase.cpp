/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "capsdatabase.h"
#include <QDataStream>
#include <QFile>
#include <QTimer>
#include <util/util.h>
#include <util/sys/paths.h>
#include "capsstorageondisk.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	CapsDatabase::CapsDatabase (const ILoadProgressReporter_ptr& lpr, QObject *parent)
	: QObject { parent }
	, Storage_ { new CapsStorageOnDisk { lpr, this } }
	{
	}

	bool CapsDatabase::Contains (const QByteArray& hash) const
	{
		if (Ver2Features_.contains (hash) && Ver2Identities_.contains (hash))
			return true;

		return Preload (hash);
	}

	QStringList CapsDatabase::Get (const QByteArray& hash) const
	{
		if (!Ver2Features_.contains (hash))
			Preload (hash);

		return Ver2Features_ [hash];
	}

	void CapsDatabase::Set (const QByteArray& hash, const QStringList& features)
	{
		Ver2Features_ [hash] = features;
		Storage_->AddFeatures (hash, features);
	}

	QList<QXmppDiscoveryIq::Identity> CapsDatabase::GetIdentities (const QByteArray& hash) const
	{
		if (!Ver2Identities_.contains (hash))
			Preload (hash);

		return Ver2Identities_ [hash];
	}

	void CapsDatabase::SetIdentities (const QByteArray& hash,
			const QList<QXmppDiscoveryIq::Identity>& ids)
	{
		Ver2Identities_ [hash] = ids;
		Storage_->AddIdentities (hash, ids);
	}

	bool CapsDatabase::Preload (const QByteArray& hash) const
	{
		const auto& features = Storage_->GetFeatures (hash);
		const auto& identities = Storage_->GetIdentities (hash);
		if (!features || !identities)
			return false;

		Ver2Features_ [hash] = *features;
		Ver2Identities_ [hash] = *identities;
		return true;
	}
}
}
}
