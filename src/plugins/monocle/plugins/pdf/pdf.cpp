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

#include "pdf.h"
#include "document.h"
#include <QIcon>

namespace LeechCraft
{
namespace Monocle
{
namespace PDF
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.PDF";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Monocle PDF";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("PDF backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	bool Plugin::CanLoadDocument (const QString& file)
	{
		return file.toLower ().endsWith (".pdf");
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		return IDocument_ptr (new Document (file));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_pdf, LeechCraft::Monocle::PDF::Plugin);
