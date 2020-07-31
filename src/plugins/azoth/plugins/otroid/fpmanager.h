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
#include <QModelIndex>

extern "C"
{
#include <libotr/proto.h>
}

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace OTRoid
{
	struct FPInfo
	{
		QString FP_;
		QString Trust_;
	};

	using FPInfos_t = QList<FPInfo>;

	class FPManager : public QObject
	{
		Q_OBJECT

		const OtrlUserState UserState_;
		IProxyObject * const AzothProxy_;
		QStandardItemModel * const Model_;

		struct EntryState
		{
			QList<QStandardItem*> EntryItems_;
			FPInfos_t FPs_;
		};

		struct AccState
		{
			QStandardItem *AccItem_ = nullptr;
			QHash<QString, EntryState> Entries_;
		};
		QHash<QString, AccState> Account2User2Fp_;

		bool ReloadScheduled_ = false;

		enum Column
		{
			ColumnEntryName,
			ColumnEntryID,
			ColumnKeysCount
		};
	public:
		enum Role
		{
			RoleType = Qt::UserRole + 1,
			RoleEntryId,
			RoleAccId,
			RoleProtoId,
			RoleSourceFP
		};

		enum Type
		{
			TypeAcc,
			TypeEntry,
			TypeFP
		};

		FPManager (const OtrlUserState, IProxyObject*, QObject* = 0);

		int HandleNew (const char*, const char*, const char*, unsigned char [20]);

		FPInfos_t GetFingerprints (const QString& accId, const QString& userId) const;

		QAbstractItemModel* GetModel () const;
	public slots:
		void reloadAll ();
		void scheduleReload ();
	private slots:
		void removeRequested (const QString&, const QModelIndexList&);
		void customButtonPressed (const QString&, const QByteArray&, int);
	signals:
		void fingerprintsChanged ();
	};
}
}
}
