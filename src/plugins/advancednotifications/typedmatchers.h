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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_TYPEDMATCHERS_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_TYPEDMATCHERS_H
#include <boost/shared_ptr.hpp>
#include <QString>
#include <QRegExp>
#include <QVariant>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	class TypedMatcherBase;
	
	typedef boost::shared_ptr<TypedMatcherBase> TypedMatcherBase_ptr;
	
	class TypedMatcherBase
	{
	public:
		static TypedMatcherBase_ptr Create (QVariant::Type);

		virtual QVariantMap Save () const = 0;
		virtual void Load (const QVariantMap&) = 0;
		
		virtual bool Match (const QVariant&) const = 0;
	};
	
	class StringLikeMatcher : public TypedMatcherBase
	{
	protected:
		QRegExp Rx_;
		bool Contains_;
	public:
		QVariantMap Save () const;
		void Load (const QVariantMap&);
	};
	
	class StringMatcher : public StringLikeMatcher
	{
	public:
		bool Match (const QVariant&) const;
	};
	
	class StringListMatcher : public StringLikeMatcher
	{
	public:
		bool Match (const QVariant&) const;
	};
	
	class IntMatcher : public TypedMatcherBase
	{
		int Boundary_;

		enum Operation
		{
			OGreater = 0x01,
			OLess = 0x02,
			OEqual = 0x04
		};
		
		Q_DECLARE_FLAGS (Operations, Operation);
		
		Operations Ops_;
	public:
		QVariantMap Save () const;
		void Load (const QVariantMap&);
		
		bool Match (const QVariant&) const;
	};
}
}

#endif
