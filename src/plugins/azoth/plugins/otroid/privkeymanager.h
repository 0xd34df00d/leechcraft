/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QAbstractItemModel>

extern "C"
{
#include <libotr/proto.h>
}

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace OTRoid
{
	class PrivKeyManager : public QObject
	{
		Q_OBJECT

		const OtrlUserState UserState_;
		IProxyObject * const AzothProxy_;
		QStandardItemModel * const Model_;

		enum Columns
		{
			ColumnAccName,
			ColumnKey
		};

		enum Roles
		{
			RoleAccId = Qt::UserRole + 1,
			RoleProtoId
		};
	public:
		PrivKeyManager (const OtrlUserState, IProxyObject*);

		QAbstractItemModel* GetModel () const;
	private:
		void GenerateRequested (int);
	public slots:
		void reloadAll ();
	private slots:
		void removeRequested (const QString&, QModelIndexList);
		void customButtonPressed (const QString&, const QByteArray&, int);
	signals:
		void keysChanged ();
		void keysGenerationRequested (const QString&, const QString&);
	};
}
}
}
