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
#include <QRectF>
#include <QMetaType>

namespace LeechCraft
{
namespace Monocle
{
	enum class LinkType
	{
		PageLink,
		URL,
		Command,
		OtherLink
	};

	class ILink
	{
	public:
		virtual ~ILink () {}

		virtual LinkType GetLinkType () const = 0;

		virtual QRectF GetArea () const = 0;

		virtual void Execute () = 0;
	};
	typedef std::shared_ptr<ILink> ILink_ptr;

	class IPageLink
	{
	public:
		virtual ~IPageLink () {}

		virtual QString GetDocumentFilename () const = 0;

		virtual int GetPageNumber () const = 0;

		virtual double NewX () const = 0;
		virtual double NewY () const = 0;
		virtual double NewZoom () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::ILink, "org.LeechCraft.Monocle.ILink/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Monocle::IPageLink, "org.LeechCraft.Monocle.IPageLink/1.0");
