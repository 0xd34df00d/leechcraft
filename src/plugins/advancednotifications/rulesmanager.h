/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/optional.hpp>
#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>
#include "notificationrule.h"

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace LC
{
struct Entity;

namespace AdvancedNotifications
{
	class RulesManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		QList<NotificationRule> Rules_;
		QStandardItemModel *RulesModel_;
	public:
		RulesManager (const ICoreProxy_ptr&, QObject* = 0);

		QAbstractItemModel* GetRulesModel () const;
		QList<NotificationRule> GetRulesList () const;
		QList<NotificationRule> GetRules (const Entity&);

		void SetRuleEnabled (const NotificationRule&, bool);
		void UpdateRule (const QModelIndex&, const NotificationRule&);

		boost::optional<NotificationRule> CreateRuleFromEntity (const Entity&);
		void HandleEntity (const Entity&);
		void SuggestRuleConfiguration (const Entity&);
		QList<Entity> GetAllRules (const QString&) const;

		void PrependRule (const NotificationRule& = {});
	private:
		void LoadDefaultRules (int = -1);
		void LoadSettings ();
		void ResetModel ();
		void SaveSettings () const;

		QList<QStandardItem*> RuleToRow (const NotificationRule&) const;
	public slots:
		void removeRule (const QModelIndex&);
		void moveUp (const QModelIndex&);
		void moveDown (const QModelIndex&);
		void setRuleEnabled (int index, bool enabled);
		void reset ();

		QVariant getRulesModel () const;
	private slots:
		void handleItemChanged (QStandardItem*);
	signals:
		void focusOnRule (const QModelIndex&);

		void rulesChanged () const;
	};
}
}
