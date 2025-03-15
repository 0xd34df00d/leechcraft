/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIdentityProxyModel>

namespace LC::Util
{
	template<typename Id>
	class SelectionProxyModel : public QIdentityProxyModel
	{
	public:
		struct Config
		{
			int IsSelectedRole_;
			int SourceIdRole_;
			const std::function<QModelIndexList (QSet<Id>)> FindItems_;
		};
	private:
		const Config Config_;

		QSet<Id> Selections_;
	public:
		explicit SelectionProxyModel (QAbstractItemModel& source, const Config& config, QObject *parent = nullptr)
		: QIdentityProxyModel { parent }
		, Config_ { config }
		{
			QIdentityProxyModel::setSourceModel (&source);
		}

		int GetIsSelectedRole () const
		{
			return Config_.IsSelectedRole_;
		}

		void SetSelections (const QSet<Id>& selections)
		{
			if (Selections_ == selections)
				return;

			EmitByIds (std::exchange (Selections_, {}));
			Selections_ = selections;
			EmitByIds (Selections_);
		}

		QVariant data (const QModelIndex& index, int role) const override
		{
			if (role != Config_.IsSelectedRole_)
				return QIdentityProxyModel::data (index, role);

			const auto id = index.data (Config_.SourceIdRole_).template value<Id> ();
			return Selections_.contains (id);
		}
	private:
		void EmitByIds (const QSet<Id>& ids)
		{
			const auto lastColumn = sourceModel ()->columnCount () - 1;
			for (const auto& idx : Config_.FindItems_ (ids))
				emit dataChanged (idx.siblingAtColumn (0), idx.siblingAtColumn (lastColumn), { Config_.IsSelectedRole_ });
		}
	};
}
