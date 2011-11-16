/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SNAILS_STORAGE_H
#define PLUGINS_SNAILS_STORAGE_H
#include <QObject>
#include <QDir>
#include <QSettings>
#include <QHash>
#include <QSet>
#include "message.h"

namespace LeechCraft
{
namespace Snails
{
	class Account;

	class Storage : public QObject
	{
		Q_OBJECT

		QDir SDir_;
		QSettings Settings_;
		QHash<QByteArray, bool> IsMessageRead_;
	public:
		Storage (QObject* = 0);

		void SaveMessages (Account*, const QList<Message_ptr>&);
		QList<Message_ptr> LoadMessages (Account*);
		Message_ptr LoadMessage (Account*, const QByteArray&);
		QSet<QByteArray> LoadIDs (Account*);
		int GetNumMessages (Account*) const;
		bool HasMessagesIn (Account*) const;

		bool IsMessageRead (Account*, const QByteArray&);
	private:
		QDir DirForAccount (Account*) const;

		void UpdateCaches (Message_ptr);
	};
}
}

#endif
