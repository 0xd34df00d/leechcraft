/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_polldialog.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class PollDialog : public QDialog
	{
		Q_OBJECT

		Ui::PollDialog Ui_;

		QStandardItemModel *CheckModel_;
		QStandardItemModel *RadioModel_;
		QStandardItemModel *DropModel_;
		QStandardItemModel *PollTypeModel_;
		bool ItemIsChanged_ = false;
		QHash<QString, QString> Type2Question_;

		enum PollCanView
		{
			ViewAll,
			ViewOnlyFriends,
			ViewOnlyMe
		};

		enum PollCanVote
		{
			VoteAll,
			VoteOnlyFriends
		};

	public:
		enum PollType
		{
			CheckBoxes,
			RadioButtons,
			DropdownBox,
			TextEntry,
			Scale
		};

		explicit PollDialog (QWidget *parent = 0);

		QString GetPollName () const;
		QString GetWhoCanView () const;
		QString GetWhoCanVote () const;
		QStringList GetPollTypes () const;
		QString GetPollQuestion (const QString& type) const;
		QVariantMap  GetPollFields (const QString& pollType) const;
		int GetScaleFrom () const;
		int GetScaleTo () const;
		int GetScaleBy () const;
		int GetTextSize () const;
		int GetTextMaxLength () const;

		void accept ();
	private:
		QVariantMap GetFields (QStandardItemModel *model) const;
		bool IsScaleValuesAreValid () const;

	private slots:
		void on_AddField__released ();
		void on_RemoveField__released ();
		void on_PollType__currentIndexChanged (int index);
		void handleItemActivated (const QModelIndex& index);
		void handleItemChanged (QStandardItem *item);
		void on_PollQuestion__textChanged (const QString& text);
	};
}
}
}
