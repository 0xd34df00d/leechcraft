/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QColor>
#include "interfaces/azoth/azothcommon.h"

template<typename>
class QList;

template<typename>
class QFuture;

class QString;
class QWidget;
class QObject;

namespace LC
{
struct Entity;

namespace Azoth
{
	class ICLEntry;
	class ISupportPGP;
	class IAccount;
	class AvatarsManager;

	Q_REQUIRED_RESULT QFuture<Entity> BuildNotification (AvatarsManager*,
			Entity, ICLEntry*, const QString& id = {}, ICLEntry* = nullptr);
	QString GetActivityIconName (const QString&, const QString&);

	void InitiateAccountAddition (QWidget *parent = 0);

	void AuthorizeEntry (ICLEntry*);
	void DenyAuthForEntry (ICLEntry*);

	QObject* FindByHRId (IAccount*, const QString&);

	QList<QColor> GenerateColors (const QString& coloring, QColor);
	QString GetNickColor (const QString& nick, const QList<QColor>& colors);

	QStringList GetMucParticipants (const QString& entryId);

	void RemoveAccount (IAccount*);

	QString StateToString (State);
	QString PrettyPrintDateTime (const QDateTime&);

	bool ChoosePGPKey (ISupportPGP*, ICLEntry*);
}
}
