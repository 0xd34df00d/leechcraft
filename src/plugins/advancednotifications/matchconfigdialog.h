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
#include <QDialog>
#include <interfaces/an/ianemitter.h>
#include "ui_matchconfigdialog.h"

namespace LC
{
struct ANFieldData;

namespace AdvancedNotifications
{
	class TypedMatcherBase;
	using TypedMatcherBase_ptr = std::shared_ptr<TypedMatcherBase>;

	class FieldMatch;

	class MatchConfigDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::AdvancedNotifications::MatchConfigDialog)

		Ui::MatchConfigDialog Ui_;

		QSet<QString> Types_;
		TypedMatcherBase_ptr CurrentMatcher_;

		const QHash<QObject*, QList<ANFieldData>> FieldsMap_;
	public:
		explicit MatchConfigDialog (const QHash<QObject*, QList<ANFieldData>>&, QWidget* = nullptr);

		FieldMatch GetFieldMatch () const;
		void SetFieldMatch (const FieldMatch&);
	private:
		int SelectPlugin (const QByteArray&, const QString&);
		void AddFields (const QList<ANFieldData>&);
		void ShowPluginFields (int);
		void ShowField (int);
	};
}
}
