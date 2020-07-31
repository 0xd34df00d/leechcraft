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
#include <QHash>
#include <QSet>
#include "compinfo.h"

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;
class QSettings;

namespace LC::Fenet
{
	class CompParamsManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const ParamsModel_;

		CompInfo CurrentInfo_;

		QHash<QString, QHash<QString, QVariant>> ChangedParams_;
		QHash<QString, QHash<QString, bool>> ChangedFlags_;
	public:
		enum Role
		{
			Description = Qt::UserRole + 1
		};

		explicit CompParamsManager (QObject* = nullptr);

		QAbstractItemModel* GetModel () const;

		void SetCompInfo (const CompInfo&);

		QStringList GetCompParams (const QString&) const;
	private:
		std::shared_ptr<QSettings> GetSettings (QString compName = QString ()) const;
	public slots:
		void handleItemChanged (QStandardItem*);

		void save ();
		void revert ();
	signals:
		void paramsChanged ();
	};
}
