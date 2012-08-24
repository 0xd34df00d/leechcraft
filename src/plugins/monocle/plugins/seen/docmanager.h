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

#pragma once

#include <memory>
#include <QObject>
#include <QHash>
#include <interfaces/monocle/ibackendplugin.h>
#include <libdjvu/ddjvuapi.h>
#include <libdjvu/miniexp.h>

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	class Document;

	class DocManager : public QObject
	{
		Q_OBJECT

		ddjvu_context_t *Context_;
		QHash<ddjvu_document_t*, std::weak_ptr<Document>> Documents_;
	public:
		DocManager (ddjvu_context_t*, QObject* = 0);

		std::shared_ptr<Document> LoadDocument (const QString&);
		void Unregister (ddjvu_document_t*);

		void HandleDocInfo (ddjvu_document_t*);
		void HandlePageInfo (ddjvu_document_t*, ddjvu_page_t*);
	};
}
}
}
