/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircjoingroupchat.h"
#include <QComboBox>
#include <QTextCodec>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcJoinGroupChat::IrcJoinGroupChat (QWidget *parent)
	: QWidget (parent)
	, SelectedAccount_ (0)
	{
		Ui_.setupUi (this);
		
		Q_FOREACH (const QByteArray& codec, QTextCodec::availableCodecs ())
			Ui_.Encoding_->addItem (QString::fromUtf8 (codec));
		Ui_.Encoding_->model ()->sort (0);
		Ui_.Encoding_->setCurrentIndex (Ui_.Encoding_->findText ("UTF-8"));
	}
	
	void IrcJoinGroupChat::AccountSelected (QObject *obj)
	{

	}
	void IrcJoinGroupChat::Join (QObject *obj)
	{

	}

	void IrcJoinGroupChat::Cancel ()
	{

	}

	QVariantList IrcJoinGroupChat::GetBookmarkedMUCs () const
	{
		return QVariantList ();
	}

	void IrcJoinGroupChat::SetIdentifyingData (const QVariantMap& data)
	{

	}

	QVariantMap IrcJoinGroupChat::GetIdentifyingData () const
	{
		return QVariantMap ();
	}
};
};
};
