/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "serverinfowidget.h"
#include <boost/bind.hpp>
#include "ircserverclentry.h"
#include <QtDebug>

namespace LeechCraft
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
		Parameter2Command_ ["casemapping"] = boost::bind (&QLineEdit::setText,
				Ui_.CaseMapping_, _1);
		Parameter2Command_ ["chanlimit"] = boost::bind (&QLineEdit::setText,
				Ui_.ChanLimit_, _1);
		Parameter2Command_ ["chanmodes"] = boost::bind (&ServerInfoWidget::SetChanModes,
				this, _1);
		Parameter2Command_ ["channellen"] = boost::bind (&QLineEdit::setText,
				Ui_.ChannelLen_, _1);
		Parameter2Command_ ["chantypes"] = boost::bind (&QLineEdit::setText,
				Ui_.ChanTypes_, _1);
		Parameter2Command_ ["excepts"] = boost::bind (&ServerInfoWidget::SetExcepts,
				this, _1);
		Parameter2Command_ ["idchan"] = boost::bind (&QLineEdit::setText,
				Ui_.IdChan_, _1);
		Parameter2Command_ ["kicklen"] = boost::bind (&QLineEdit::setText,
				Ui_.KickLen_, _1);
		Parameter2Command_ ["maxlist"] = boost::bind (&QLineEdit::setText,
				Ui_.MaxList_, _1);
		Parameter2Command_ ["modes"] = boost::bind (&QLineEdit::setText,
				Ui_.Modes_, _1);
		Parameter2Command_ ["network"] = boost::bind (&QLineEdit::setText,
				Ui_.NetworkName_, _1);
		Parameter2Command_ ["nicklen"] = boost::bind (&QLineEdit::setText,
				Ui_.NickLength_, _1);
		Parameter2Command_ ["prefix"] = boost::bind (&ServerInfoWidget::SetPrefix,
				this, _1);
		Parameter2Command_ ["safelist"] = boost::bind (&ServerInfoWidget::SetSafeList,
				this, _1);
		Parameter2Command_ ["statusmsg"] = boost::bind (&QLineEdit::setText,
				Ui_.StatusMsg_, _1);
		Parameter2Command_ ["std"] = boost::bind (&QLineEdit::setText,
				Ui_.Std_, _1);
		Parameter2Command_ ["targmax"] = boost::bind (&ServerInfoWidget::SetTargMax,
				this, _1);
		Parameter2Command_ ["topiclen"] = boost::bind (&QLineEdit::setText,
				Ui_.TopicLen_, _1);
		Parameter2Command_ ["invex"] = boost::bind (&ServerInfoWidget::SetInvEx,
				this, _1);
	}

	void ServerInfoWidget::SetISupport ()
	{
		const QMap<QString, QString>& info = ISCLEntry_->GetISupport ();

		for (QMap<QString, QString>::const_iterator it_begin = info.begin (),
				it_end = info.end (); it_begin != it_end; ++it_begin)
			if (Parameter2Command_.contains (it_begin.key ().toLower ()))
				Parameter2Command_ [it_begin.key ().toLower ()] (it_begin.value ());
	}

	void ServerInfoWidget::SetChanModes (const QString& modes)
	{
		const QStringList& list = modes.split (',');

		Ui_.ChanModesA_->setText (list.at (0));
		Ui_.ChanModesB_->setText (list.at (1));
		Ui_.ChanModesC_->setText (list.at (2));
		Ui_.ChanModesD_->setText (list.at (3));
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
			QTableWidgetItem *newMode = new QTableWidgetItem (modeStr [i]);
			QTableWidgetItem *newPrefix = new QTableWidgetItem (prefixStr [i]);
			Ui_.tableWidget->setItem (i, 0, newMode);
			Ui_.tableWidget->setItem (i, 1, newPrefix);
		}
	}

	void ServerInfoWidget::SetSafeList (const QString& str)
	{
		Ui_.SafeList_->setChecked (GetBoolFromString (str));
	}

	void ServerInfoWidget::SetTargMax (const QString& str)
	{
		const QStringList& list = str.split (',');

		Ui_.TargetMax_->clear ();
		Ui_.TargetMax_->setRowCount (list.count ());

		int row = 0;
		Q_FOREACH (const QString& param, list)
		{
			const int index = param.indexOf (':');
			QTableWidgetItem *target = new QTableWidgetItem (param.mid (0, index));
			QTableWidgetItem *count = new QTableWidgetItem (param.mid (index + 1));
			Ui_.TargetMax_->setItem (row, 0, target);
			Ui_.TargetMax_->setItem (row, 1, count);
			++row;
		}
	}

	void ServerInfoWidget::SetInvEx (const QString& str)
	{
		Ui_.InvEx_->setChecked (GetBoolFromString (str));
	}

	bool ServerInfoWidget::GetBoolFromString (const QString& str)
	{
		return str == "true";
	}

	void ServerInfoWidget::accept ()
	{
	}
}
}
}
