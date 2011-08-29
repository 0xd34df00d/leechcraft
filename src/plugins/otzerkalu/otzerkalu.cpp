/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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

#include "otzerkalu.h"
#include <QIcon>
#include <interfaces/entitytesthandleresult.h>
#include "otzerkaludialog.h"

namespace LeechCraft
{
namespace Otzerkalu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Otzerkalu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Otzerkalu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Otzerkalu allows one to recursively download a web site.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		const bool can = !entity.Entity_.toUrl ().isEmpty () &&
				(entity.Parameters_ & FromUserInitiated) &&
				entity.Additional_.value ("AllowedSemantics").toStringList ().contains ("save");
		return can ?
				EntityTestHandleResult (EntityTestHandleResult::PHigh) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity entity)
	{
		QUrl dUrl = entity.Entity_.toUrl ();
		if (!dUrl.isValid ())
			return;
		
		OtzerkaluDialog dialog;
		if (dialog.exec () != QDialog::Accepted)
			return;

		OtzerkaluDownloader *dl = new OtzerkaluDownloader (DownloadParams (dUrl, dialog.GetDir (),
					dialog.GetRecursionLevel (),
					dialog.FetchFromExternalHosts ()),
					this);
		
		connect (dl,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (dl,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
		
		dl->Begin ();
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_otzerkalu, LeechCraft::Otzerkalu::Plugin);
