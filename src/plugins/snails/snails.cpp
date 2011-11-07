/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "snails.h"
#include <QIcon>
#include "mailtab.h"

namespace LeechCraft
{
namespace Snails
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		MailTabClass_.TabClass_ = "mail";
		MailTabClass_.VisibleName_ = tr ("Mail");
		MailTabClass_.Description_ = tr ("Mail tab.");
		MailTabClass_.Icon_ = GetIcon ();
		MailTabClass_.Priority_ = 55;
		MailTabClass_.Features_ = TFOpenableByRequest;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Snails";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Snails";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LeechCraft mail client.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t result;
		result << MailTabClass_;
		return result;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "mail")
		{
			MailTab *mt = new MailTab (MailTabClass_, this);

			connect (mt,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			emit addNewTab (MailTabClass_.VisibleName_, mt);
			emit raiseTab (mt);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_snails, LeechCraft::Snails::Plugin);

