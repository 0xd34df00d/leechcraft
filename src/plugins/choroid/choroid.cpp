/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 * Copyright (C) 2011 ForNeVeR
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

#include "choroid.h"
#include <QIcon>
#include "choroidtab.h"

namespace LeechCraft
{
namespace Choroid
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		TabInfo_.TabClass_ = "ChoroidTab";
		TabInfo_.VisibleName_ = "Choroid";
		TabInfo_.Description_ = tr ("Image viewer tab");
		TabInfo_.Icon_ = GetIcon ();
		TabInfo_.Priority_ = 50;
		TabInfo_.Features_ = TFOpenableByRequest;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Choroid";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Choroid";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Image viewer for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t result;
		result << TabInfo_;
		return result;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "ChoroidTab")
		{
			ChoroidTab *t = new ChoroidTab (TabInfo_, this);

			connect (t,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			emit addNewTab ("Choroid", t);
			emit raiseTab (t);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_choroid, LeechCraft::Choroid::Plugin);
