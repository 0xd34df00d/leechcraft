/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIdentityProxyModel>
#include <QSet>
#include "modelsconfig.h"

namespace LC::Util
{
	class UTIL_MODELS_API CheckableProxyModelBase : public QIdentityProxyModel
	{
		Q_OBJECT
	protected:
		const int IdRole_;
	public:
		explicit CheckableProxyModelBase (int idRole, QObject *parent = nullptr);

		Qt::ItemFlags flags (const QModelIndex& index) const override;
		QVariant data (const QModelIndex& index, int role) const override;
		bool setData (const QModelIndex& index, const QVariant& value, int role) override;

		void CheckAll ();
		void CheckNone ();
	protected:
		virtual bool IsChecked (const QVariant& idVar) const = 0;
		virtual void SetChecked (const QVariant& idVar, bool checked) = 0;
	signals:
		void selectionChanged ();
	};

	template<typename IdType>
	class CheckableProxyModel : public CheckableProxyModelBase
	{
		QSet<IdType> Unchecked_;
	public:
		using CheckableProxyModelBase::CheckableProxyModelBase;

		QSet<IdType> GetUnchecked () const
		{
			return Unchecked_;
		}

		QSet<IdType> GetChecked () const
		{
			const auto rc = sourceModel ()->rowCount ();

			QSet<IdType> result;
			result.reserve (rc - Unchecked_.size ());

			for (int i = 0; i < rc; ++i)
			{
				const auto rowId = sourceModel ()->index (i, 0).data (IdRole_).template value<IdType> ();
				if (!Unchecked_.contains (rowId))
					result << rowId;
			}

			return result;
		}
	protected:
		bool IsChecked (const QVariant& idVar) const override
		{
			return !Unchecked_.contains (idVar.value<IdType> ());
		}

		void SetChecked (const QVariant& idVar, bool checked) override
		{
			const auto id = idVar.value<IdType> ();
			if (checked)
				Unchecked_.remove (id);
			else
				Unchecked_ << id;
		}
	};
}
