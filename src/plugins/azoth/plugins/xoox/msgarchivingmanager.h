/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

namespace LeechCraft
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
		bool Valid_;
		bool AutoSave_;
		MsgArchSetting Default_;
		QMap<MsgArchMethod, MsgArchMethodPolicy> MethodPolicies_;

		QMap<QString, MsgArchSetting> ItemSettings_;

		MsgArchPrefs ();
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
