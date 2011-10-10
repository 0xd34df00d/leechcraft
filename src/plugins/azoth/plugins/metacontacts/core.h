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

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_CORE_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_CORE_H
#include <QObject>
#include <QHash>

namespace LeechCraft
{
namespace Azoth
{
class ICLEntry;

namespace Metacontacts
{
	class MetaAccount;
	class MetaEntry;

	class Core : public QObject
	{
		Q_OBJECT

		bool SaveEntriesScheduled_;

		MetaAccount *Account_;
		QList<MetaEntry*> Entries_;

		QHash<QString, MetaEntry*> UnavailRealEntries_;
		QHash<QString, MetaEntry*> AvailRealEntries_;

		Core ();
	public:
		static Core& Instance ();

		void SetMetaAccount (MetaAccount*);
		QList<QObject*> GetEntries () const;

		bool HandleRealEntryAddBegin (QObject*);
		void AddRealEntry (QObject*);

		bool HandleDnDEntry2Entry (QObject*, QObject*);

		void RemoveEntry (MetaEntry*);

		void ScheduleSaveEntries ();
	private:
		void AddRealToMeta (MetaEntry*, ICLEntry*);
		MetaEntry* CreateMetaEntry ();
		void ConnectSignals (MetaEntry*);
	private slots:
		void handleEntriesRemoved (const QList<QObject*>&);
		void handleEntryShouldBeRemoved ();
		void saveEntries ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
	};
}
}
}

#endif
