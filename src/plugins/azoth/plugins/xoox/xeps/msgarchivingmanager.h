/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	enum class MsgArchOTR
	{
		Approve,
		Concede,
		Forbid,
		Oppose,
		Prefer,
		Require
	};

	enum class MsgArchSave
	{
		Body,
		False,
		Message,
		Stream
	};

	enum class MsgArchMethod
	{
		Auto,
		Local,
		Manual
	};

	bool operator< (MsgArchMethod, MsgArchMethod);

	enum class MsgArchMethodPolicy
	{
		Concede,
		Forbid,
		Prefer
	};

	struct MsgArchSetting
	{
		MsgArchOTR OTR_;
		MsgArchSave Save_;
		qint64 Expire_;
	};

	struct MsgArchPrefs
	{
		bool Valid_ = false;
		bool AutoSave_ = false;
		MsgArchSetting Default_;
		QMap<MsgArchMethod, MsgArchMethodPolicy> MethodPolicies_;

		QMap<QString, MsgArchSetting> ItemSettings_;
	};

	class MsgArchivingManager : public QXmppClientExtension
	{
		Q_OBJECT

		ClientConnection *Conn_;
		MsgArchPrefs Prefs_;
	public:
		MsgArchivingManager (ClientConnection*);

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);

		void RequestPrefs ();
		MsgArchPrefs GetPrefs () const;

		void SetArchSetting (const MsgArchSetting& setting,
				const QString& jid = QString ());
		void SetMethodPolicies (const QMap<MsgArchMethod, MsgArchMethodPolicy>&);
	private:
		void HandlePref (const QDomElement&);
	signals:
		void archPreferencesChanged ();
	};
}
}
}
