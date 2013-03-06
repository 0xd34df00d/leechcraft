/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <memory>
#include <QImage>
#include <QMetaType>
#include <QStringList>
#include <QDateTime>
#include "ilink.h"

namespace LeechCraft
{
namespace Monocle
{
	struct DocumentInfo
	{
		QString Title_;
		QString Subject_;
		QString Author_;

		QStringList Genres_;
		QStringList Keywords_;

		QDateTime Date_;
	};

	class IDocument
	{
	public:
		virtual ~IDocument () {}

		virtual QObject* GetObject () = 0;

		virtual bool IsValid () const = 0;

		virtual DocumentInfo GetDocumentInfo () const = 0;

		virtual int GetNumPages () const = 0;

		virtual QSize GetPageSize (int) const = 0;

		virtual QImage RenderPage (int, double xRes, double yRes) = 0;

		virtual QList<ILink_ptr> GetPageLinks (int) = 0;
	protected:
		virtual void navigateRequested (const QString&, int pageNum, double x, double y) = 0;
	};

	typedef std::shared_ptr<IDocument> IDocument_ptr;
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IDocument,
		"org.LeechCraft.Monocle.IDocument/1.0");
