/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QXmppDiscoveryIq.h>
#include <interfaces/core/iloadprogressreporter.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class CapsStorageOnDisk;

	class CapsDatabase : public QObject
	{
		mutable QHash<QByteArray, QStringList> Ver2Features_;
		mutable QHash<QByteArray, QList<QXmppDiscoveryIq::Identity>> Ver2Identities_;

		CapsStorageOnDisk * const Storage_;
	public:
		CapsDatabase (const ILoadProgressReporter_ptr&, QObject* = 0);

		bool Contains (const QByteArray&) const;

		QStringList Get (const QByteArray&) const;
		void Set (const QByteArray&, const QStringList&);

		QList<QXmppDiscoveryIq::Identity> GetIdentities (const QByteArray&) const;
		void SetIdentities (const QByteArray&, const QList<QXmppDiscoveryIq::Identity>&);
	private:
		bool Preload (const QByteArray&) const;
	};
}
}
}
