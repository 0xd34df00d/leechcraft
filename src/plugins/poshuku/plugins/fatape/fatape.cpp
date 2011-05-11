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

#include "fatape.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <QDir>
#include <QIcon>
#include <QStringList>
#include <plugininterface/util.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	template<typename Iter, typename Pred, typename Func>
	void apply_if (Iter first, Iter last, Pred pred, Func func)
	{
		for (; first != last; ++first)
			if (pred (*first))
				func (*first);
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
	}
	
	void Plugin::SecondInit ()
	{
		QDir scriptsDir (Util::CreateIfNotExists ("data/poshuku/fatape/scripts"));

		if (!scriptsDir.exists ())
			return;
		
		QStringList filter ("*.user.js");

		Q_FOREACH (const QString& script, scriptsDir.entryList (filter, QDir::Files))
			UserScripts_.append (UserScript (scriptsDir.absoluteFilePath (script)));
	}
	
	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.FatApe";
	}
	
	QString Plugin::GetName () const
	{
		return "Poshuku FatApe";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("GreaseMonkey support layer for the Poshuku browser.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void Plugin::hookInitialLayoutCompleted (LeechCraft::IHookProxy_ptr proxy, 
			QWebPage *page, QWebFrame *frame)
	{
		boost::function<bool (const UserScript&)> match = 
			boost::bind (&UserScript::MatchToPage, 
					_1,
					frame->url ().toString ());
		boost::function<void (const UserScript&)> inject = 
			boost::bind (&UserScript::Inject,
					_1,
					frame);

		apply_if (UserScripts_.begin (), UserScripts_.end (), match, inject);
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_fatape, LeechCraft::Poshuku::FatApe::Plugin);
