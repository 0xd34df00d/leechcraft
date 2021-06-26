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
		Q_DECLARE_TR_FUNCTIONS (LC::AdvancedNotifications::NotificationRulesWidget)

		Ui::NotificationRulesWidget Ui_;

		RulesManager * const RM_;
		const AudioThemeManager * const AudioThemeManager_;
		const UnhandledNotificationsKeeper * const UnhandledKeeper_;

		FieldMatches_t Matches_;
		QStandardItemModel *MatchesModel_;
	public:
		NotificationRulesWidget (RulesManager*,
				const AudioThemeManager*,
				const UnhandledNotificationsKeeper*,
				QWidget* = nullptr);
	private:
		void ResetMatchesModel ();

		QString GetCurrentCat () const;
		QStringList GetSelectedTypes () const;

		NotificationRule GetRuleFromUI (QModelIndex = {}) const;
		QList<QStandardItem*> MatchToRow (const FieldMatch&) const;

		QHash<QObject*, QList<ANFieldData>> GetRelevantANFieldsWPlugins () const;
		QList<ANFieldData> GetRelevantANFields () const;
		QString GetArgumentText ();

		void HandleItemSelected (const QModelIndex&, const QModelIndex&);

		void AddFromMissed ();
		void ResetRules ();

		void AddMatch ();
		void ModifyMatch ();
		void RemoveMatch ();

		void BrowseAudioFile ();
		void TestAudio ();

		void AddArgument ();
		void ModifyArgument ();
		void RemoveArgument ();

		void PopulateCategories ();
		void ResetAudioFileBox ();
	};
}
}
