/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin  <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>
#include "interfaces/blogique/iaccount.h"
#include "ui_exportwizard.h"

class QStandardItemModel;

namespace LC
{
namespace Blogique
{
	class IAccount;

	class ExportWizard : public QWizard
	{
		Q_OBJECT

		Ui::ExportWizard Ui_;

		QMap<int, IAccount*> Id2Account_;
		QMap<IAccount*, QStringList> Account2Tags_;
		QStandardItemModel *AllTagsModel_;
		QStandardItemModel *SelectedTagsModel_;
		QButtonGroup *Formats_;
		QMap<int, QRadioButton*> Id2RadioButton_;

		QList<Entry> Entries_;

		enum ExportFormat
		{
			PlainText,
			Html,
			Fb2,
			Pdf
		};

		enum Pages
		{
			WelcomePage,
			FormatPage,
			ContentPage,
			OverviewPage,
			ExportPage
		};
	public:
		explicit ExportWizard (QWidget *parent = nullptr);
		bool validateCurrentPage ();
		void reject ();
	private:
		void FillTags (IAccount *acc);

	private slots:
		void handleAccountChanged (int index);
		void handleCurrentIdChanged (int id);
		void selectExportPath ();
		void addTag ();
		void removeTag();

		void on_AccountSelection__currentIndexChanged (int index);
		void on_SavePath__textChanged (const QString& text);
		void on_FromDate__dateChanged (const QDate& date);
		void on_TillDate__dateChanged (const QDate& date);

		void handleTagsUpdated (const QHash<QString, int>& tags);
		void handleGotFilteredEntries (const QList<Entry>& entries);
		void handleGettingFilteredEntriesFinished ();
	};
}
}
