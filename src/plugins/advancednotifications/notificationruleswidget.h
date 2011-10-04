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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_NOTIFICATIONRULESWIDGET_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_NOTIFICATIONRULESWIDGET_H
#include <QWidget>
#include <QList>
#include "ui_notificationruleswidget.h"
#include "notificationrule.h"

class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class NotificationRulesWidget : public QWidget
	{
		Q_OBJECT

		Ui::NotificationRulesWidget Ui_;

		QMap<QString, QString> Cat2HR_;
		QMap<QString, QString> Type2HR_;
		QMap<QString, QStringList> Cat2Types_;

		QList<NotificationRule> Rules_;
		QStandardItemModel *RulesModel_;

		FieldMatches_t Matches_;
		QStandardItemModel *MatchesModel_;
	public:
		NotificationRulesWidget (QWidget* = 0);

		QList<NotificationRule> GetRules () const;
		void SetRuleEnabled (const NotificationRule&, bool);
	private:
		void LoadDefaultRules ();
		void LoadSettings ();
		void ResetModel ();
		void ResetMatchesModel ();
		QStringList GetSelectedTypes () const;
		NotificationRule GetRuleFromUI () const;
		QList<QStandardItem*> RuleToRow (const NotificationRule&) const;
		QList<QStandardItem*> MatchToRow (const FieldMatch&) const;
		void SaveSettings () const;
	private slots:
		void handleItemSelected (const QModelIndex&);
		void handleItemChanged (QStandardItem*);

		void on_AddRule__released ();
		void on_UpdateRule__released ();
		void on_MoveRuleUp__released ();
		void on_MoveRuleDown__released ();
		void on_RemoveRule__released ();
		void on_DefaultRules__released ();

		void on_AddMatch__released ();
		void on_ModifyMatch__released ();
		void on_RemoveMatch__released ();

		void on_EventCat__activated (int);

		void on_NotifyVisual__stateChanged (int);
		void on_NotifySysTray__stateChanged (int);
		void on_NotifyAudio__stateChanged (int);
		void on_NotifyCmd__stateChanged (int);

		void on_BrowseAudioFile__released ();
		void on_TestAudio__released ();

		void on_AddArgument__released ();
		void on_ModifyArgument__released ();
		void on_RemoveArgument__released ();

		void resetAudioFileBox ();
	};
}
}

#endif
