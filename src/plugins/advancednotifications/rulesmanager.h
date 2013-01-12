/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QObject>
#include "notificationrule.h"

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class RulesManager : public QObject
	{
		Q_OBJECT

		QList<NotificationRule> Rules_;
		QStandardItemModel *RulesModel_;

		QMap<QString, QString> Cat2HR_;
		QMap<QString, QString> Type2HR_;
	public:
		RulesManager (QObject* = 0);

		QAbstractItemModel* GetRulesModel () const;
		QList<NotificationRule> GetRulesList () const;

		const QMap<QString, QString>& GetCategory2HR () const;
		const QMap<QString, QString>& GetType2HR () const;

		void SetRuleEnabled (const NotificationRule&, bool);
		void UpdateRule (const QModelIndex&, const NotificationRule&);
	private:
		void LoadDefaultRules (int = -1);
		void LoadSettings ();
		void ResetModel ();
		void SaveSettings () const;

		QList<QStandardItem*> RuleToRow (const NotificationRule&) const;
	public slots:
		void prependRule ();
		void removeRule (const QModelIndex&);
		void moveUp (const QModelIndex&);
		void moveDown (const QModelIndex&);
		void setRuleEnabled (int index, bool enabled);
		void reset ();

		QVariant getRulesModel () const;
	private slots:
		void handleItemChanged (QStandardItem*);
	};
}
}
