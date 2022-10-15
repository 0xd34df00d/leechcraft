/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_JOINCONFERENCEDIALOG_H
#define PLUGINS_AZOTH_JOINCONFERENCEDIALOG_H
#include <QDialog>
#include "interfaces/azoth/iaccount.h"
#include "ui_joinconferencedialog.h"

namespace LC
{
namespace Azoth
{
	class JoinConferenceDialog : public QDialog
	{
		Q_OBJECT

		Ui::JoinConferenceDialog Ui_;
		QHash<IProtocol*, QWidget*> Proto2Joiner_;
	public:
		JoinConferenceDialog (const QList<IAccount*>&, QWidget* = 0);
		virtual ~JoinConferenceDialog ();
		
		void SetIdentifyingData (const QVariantMap&);
	public slots:
		virtual void accept ();
		virtual void reject ();
	private slots:
		void on_AccountBox__currentIndexChanged (int);
		void on_BookmarksBox__activated (int);
		void on_HistoryBox__activated (int);
		void handleValidityChanged (bool);
	private:
		void FillWidget (const QVariantMap&);
	};
}
}

#endif
