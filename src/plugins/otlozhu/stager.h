/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QDir>
#include <QVariantMap>
#include <laretz/item.h>

namespace Laretz
{
	class Operation;
	class OpSummer;
}

namespace LC
{
namespace Util
{
namespace Sync
{
	Laretz::Field_t ToField (const QString&);
	Laretz::Field_t ToField (const QStringList&);
	Laretz::Field_t ToField (const QDateTime&);
	Laretz::Field_t ToField (const QVariant&);

	template<typename T>
	Laretz::Field_t ToField (T t)
	{
		return t;
	}

	void FillItem (Laretz::Item&, const QVariantMap&);
	QVariantMap ItemToMap (const Laretz::Item&);

	class Stager : public QObject
	{
		Q_OBJECT

		QDir StagingDir_;

		std::shared_ptr<Laretz::OpSummer> Summer_;

		bool IsEnabled_;
	public:
		Stager (const QString& areaId, QObject* = 0);

		void Enable ();
		bool IsEnabled () const;

		typedef std::shared_ptr<void> MergeGuard_t;
		MergeGuard_t EnterMergeMode ();

		void Add (const std::vector<Laretz::Operation>&);
		QList<Laretz::Operation> GetStagedOps () const;
	};
}
}
}
