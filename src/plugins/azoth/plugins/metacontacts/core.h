/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_CORE_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_CORE_H
#include <QObject>
#include <QHash>

namespace LC
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

		bool SaveEntriesScheduled_ = false;

		MetaAccount *Account_ = nullptr;
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

		void HandleEntriesRemoved (const QList<QObject*>&, bool readd);
	private:
		void AddRealToMeta (MetaEntry*, ICLEntry*);
		MetaEntry* CreateMetaEntry ();
		void ConnectSignals (MetaEntry*);
	private slots:
		void handleEntryShouldBeRemoved ();
		void saveEntries ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);

		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}

#endif
