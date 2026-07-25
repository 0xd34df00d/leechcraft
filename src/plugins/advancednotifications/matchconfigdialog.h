/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QSet>
#include <QHash>
#include <interfaces/an/ianemitter.h>
#include "fieldmatch.h"
#include "matchconfigwidget.h"
#include "ui_matchconfigdialog.h"

namespace LC::AN
{
	struct FieldData;
}

namespace LC::AdvancedNotifications
{
	struct FieldMatch;

	class MatchConfigDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::AdvancedNotifications::MatchConfigDialog)

		Ui::MatchConfigDialog Ui_;

		QSet<QString> Types_;
		std::variant<IConfigWidget_ptr, QLabel> CurrentConfigWidget_;

		const QHash<QObject*, QList<AN::FieldData>> FieldsMap_;
	public:
		explicit MatchConfigDialog (const QHash<QObject*, QList<AN::FieldData>>&, QWidget* = nullptr);

		std::optional<FieldMatch> GetFieldMatch () const;
		void SetFieldMatch (const FieldMatch&);
	private:
		int SelectPlugin (const QByteArray&, const QString&);
		void AddFields (const QList<AN::FieldData>&);
		void ShowPluginFields (int);
		void ShowField (int, const std::optional<ValueMatcherOrData>& = {});
	};
}
