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
#include <QtPlugin>

namespace LeechCraft
{
namespace Monocle
{
	enum class AnnotationType
	{
		Text,
		Highlight
	};

	class IAnnotation
	{
	public:
		virtual ~IAnnotation () {}

		virtual QString GetAuthor () const = 0;

		virtual QDateTime GetDate () const = 0;

		virtual AnnotationType GetAnnotationType () const = 0;
	};

	class ITextAnnotation : public IAnnotation
	{
	public:
		virtual ~ITextAnnotation () {}

		virtual QString GetText () const = 0;

		virtual bool IsInline () const = 0;
	};

	typedef std::shared_ptr<IAnnotation> IAnnotation_ptr;
}
}

Q_DECLARE_INTERFACE (LeechCraft::Monocle::IAnnotation,
		"org.LeechCraft.Monocle.IAnnotation/1.0");
Q_DECLARE_INTERFACE (LeechCraft::Monocle::ITextAnnotation,
		"org.LeechCraft.Monocle.ITextAnnotation/1.0");
