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
#include <QDialog>
#include <interfaces/an/ianemitter.h>
#include "ui_matchconfigdialog.h"

namespace LC
{
struct ANFieldData;

namespace AdvancedNotifications
{
	class TypedMatcherBase;
	typedef std::shared_ptr<TypedMatcherBase> TypedMatcherBase_ptr;

	class FieldMatch;

	class MatchConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::MatchConfigDialog Ui_;

		QSet<QString> Types_;
		TypedMatcherBase_ptr CurrentMatcher_;

		QMap<QObject*, QList<ANFieldData>> FieldsMap_;
	public:
		MatchConfigDialog (const QMap<QObject*, QList<ANFieldData>>&, QWidget* = 0);

		FieldMatch GetFieldMatch () const;
		void SetFieldMatch (const FieldMatch&);
	private:
		int SelectPlugin (const QByteArray&, const QString&);
		void AddFields (const QList<ANFieldData>&);
	private slots:
		void on_SourcePlugin__activated (int);
		void on_FieldName__activated (int);
	};
}
}
