/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverinfowidget.h"
#include <QtDebug>
#include <util/sll/functional.h>
#include <util/sll/qtutil.h>
#include "ircserverclentry.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ServerInfoWidget::ServerInfoWidget (IrcServerCLEntry *isEntry , QWidget *parent)
	: QWidget (parent)
	, ISCLEntry_ (isEntry)
	{
		Ui_.setupUi (this);
		Init ();
		SetISupport ();
	}

	void ServerInfoWidget::Init ()
	{
		using Util::BindMemFn;

		Parameter2Command_ ["casemapping"] = BindMemFn (&QLineEdit::setText, Ui_.CaseMapping_);
		Parameter2Command_ ["chanlimit"] = BindMemFn (&QLineEdit::setText, Ui_.ChanLimit_);
		Parameter2Command_ ["chanmodes"] = BindMemFn (&ServerInfoWidget::SetChanModes, this);
		Parameter2Command_ ["channellen"] = BindMemFn (&QLineEdit::setText, Ui_.ChannelLen_);
		Parameter2Command_ ["chantypes"] = BindMemFn (&QLineEdit::setText, Ui_.ChanTypes_);
		Parameter2Command_ ["excepts"] = BindMemFn (&ServerInfoWidget::SetExcepts, this);
		Parameter2Command_ ["idchan"] = BindMemFn (&QLineEdit::setText, Ui_.IdChan_);
		Parameter2Command_ ["kicklen"] = BindMemFn (&QLineEdit::setText, Ui_.KickLen_);
		Parameter2Command_ ["maxlist"] = BindMemFn (&QLineEdit::setText, Ui_.MaxList_);
		Parameter2Command_ ["modes"] = BindMemFn (&QLineEdit::setText, Ui_.Modes_);
		Parameter2Command_ ["network"] = BindMemFn (&QLineEdit::setText, Ui_.NetworkName_);
		Parameter2Command_ ["nicklen"] = BindMemFn (&QLineEdit::setText, Ui_.NickLength_);
		Parameter2Command_ ["prefix"] = BindMemFn (&ServerInfoWidget::SetPrefix, this);
		Parameter2Command_ ["safelist"] = BindMemFn (&ServerInfoWidget::SetSafeList, this);
		Parameter2Command_ ["statusmsg"] = BindMemFn (&QLineEdit::setText, Ui_.StatusMsg_);
		Parameter2Command_ ["std"] = BindMemFn (&QLineEdit::setText, Ui_.Std_);
		Parameter2Command_ ["targmax"] = BindMemFn (&ServerInfoWidget::SetTargMax, this);
		Parameter2Command_ ["topiclen"] = BindMemFn (&QLineEdit::setText, Ui_.TopicLen_);
		Parameter2Command_ ["invex"] = BindMemFn (&ServerInfoWidget::SetInvEx, this);
	}

	void ServerInfoWidget::SetISupport ()
	{
		for (const auto& pair : Util::Stlize (ISCLEntry_->GetISupport ()))
		{
			const auto& key = pair.first.toLower ();
			if (Parameter2Command_.contains (key))
				Parameter2Command_ [key] (pair.second);
		}
	}

	void ServerInfoWidget::SetChanModes (const QString& modes)
	{
		const auto& list = modes.split (',');

		Ui_.ChanModesA_->setText (list.at (0));
		Ui_.ChanModesB_->setText (list.at (1));
		Ui_.ChanModesC_->setText (list.at (2));
		Ui_.ChanModesD_->setText (list.at (3));
	}

	namespace
	{
		bool GetBoolFromString (const QString& str)
		{
			return str == "true";
		}
	}

	void ServerInfoWidget::SetExcepts (const QString& str)
	{
		Ui_.Excepts_->setChecked (GetBoolFromString (str));
	}

	void ServerInfoWidget::SetPrefix (const QString& str)
	{
		const int mode_end = str.indexOf (')');
		const QString& modeStr = str.mid (1, mode_end - 1);
		const QString& prefixStr = str.mid (mode_end + 1);
		const int rowCount = qMin (modeStr.length (), prefixStr.length ());

		if (modeStr.length () != prefixStr.length ())
			qWarning () << "number of modes is not equal to number of prefixes";

		Ui_.tableWidget->clear ();
		Ui_.tableWidget->setRowCount (rowCount);

		for (int i = 0; i < rowCount; ++i)
		{
			Ui_.tableWidget->setItem (i, 0, new QTableWidgetItem (modeStr [i]));
			Ui_.tableWidget->setItem (i, 1, new QTableWidgetItem (prefixStr [i]));
		}
	}

	void ServerInfoWidget::SetSafeList (const QString& str)
	{
		Ui_.SafeList_->setChecked (GetBoolFromString (str));
	}

	void ServerInfoWidget::SetTargMax (const QString& str)
	{
		const auto& list = str.split (',');

		Ui_.TargetMax_->clear ();
		Ui_.TargetMax_->setRowCount (list.count ());

		int row = 0;
		for (const auto& param : list)
		{
			const int index = param.indexOf (':');
			Ui_.TargetMax_->setItem (row, 0, new QTableWidgetItem (param.mid (0, index)));
			Ui_.TargetMax_->setItem (row, 1, new QTableWidgetItem (param.mid (index + 1)));
			++row;
		}
	}

	void ServerInfoWidget::SetInvEx (const QString& str)
	{
		Ui_.InvEx_->setChecked (GetBoolFromString (str));
	}

	void ServerInfoWidget::accept ()
	{
	}
}
}
}
