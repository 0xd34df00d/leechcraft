/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "keeso.h"
#include <QIcon>

namespace LeechCraft
{
namespace Azoth
{
namespace Keeso
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Keeso";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Keeso";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to WrItE lIkE tHiS easily.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/azoth/keeso/resources/images/keeso.svg");
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	void Plugin::hookMessageWillCreated (IHookProxy_ptr proxy,
			QObject*, QObject*, int, QString)
	{
		QString text = proxy->GetValue ("text").toString ();
		if (!text.startsWith ("/keeso "))
			return;

		text = text.mid (QString ("/keeso ").length ()).trimmed ();
		bool isUpper = qrand () % 2;
		for (int i = 0, length = text.length (); i < length; ++i)
		{
			QChar c = text.at (i);
			const QChar& u = c.toUpper ();
			const QChar& l = c.toLower ();
			if (u == l)
				continue;

			text [i] = isUpper ? u : l;
			isUpper = (qrand () % 4) ? !isUpper : isUpper;
		}

		proxy->SetValue ("text", text);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_keeso, LeechCraft::Azoth::Keeso::Plugin);
