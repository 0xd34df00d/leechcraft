/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QList>
#include <interfaces/core/icoreproxyfwd.h>
#include "ui_notificationruleswidget.h"
#include "notificationrule.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
struct ANFieldData;

namespace AdvancedNotifications
{
	class RulesManager;
	class AudioThemeManager;
	class UnhandledNotificationsKeeper;

	class NotificationRulesWidget : public QWidget
	{
		Q_OBJECT

		Ui::NotificationRulesWidget Ui_;

		RulesManager * const RM_;
		const AudioThemeManager * const AudioThemeManager_;
		const UnhandledNotificationsKeeper * const UnhandledKeeper_;
		const ICoreProxy_ptr Proxy_;

		FieldMatches_t Matches_;
		QStandardItemModel *MatchesModel_;
	public:
		NotificationRulesWidget (RulesManager*, const AudioThemeManager*,
				const UnhandledNotificationsKeeper*, const ICoreProxy_ptr&, QWidget* = 0);
	private:
		void ResetMatchesModel ();

		QString GetCurrentCat () const;
		QStringList GetSelectedTypes () const;

		NotificationRule GetRuleFromUI (QModelIndex = {}) const;
		QList<QStandardItem*> MatchToRow (const FieldMatch&) const;

		QMap<QObject*, QList<ANFieldData>> GetRelevantANFieldsWPlugins () const;
		QList<ANFieldData> GetRelevantANFields () const;
		QString GetArgumentText ();
	private slots:
		void handleItemSelected (const QModelIndex&, const QModelIndex&);
		void selectRule (const QModelIndex&);

		void on_AddRule__released ();
		void on_AddFromMissed__released ();
		void on_UpdateRule__released ();
		void on_MoveRuleUp__released ();
		void on_MoveRuleDown__released ();
		void on_RemoveRule__released ();
		void on_DefaultRules__released ();

		void on_AddMatch__released ();
		void on_ModifyMatch__released ();
		void on_RemoveMatch__released ();

		void on_EventCat__currentIndexChanged (int);

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
