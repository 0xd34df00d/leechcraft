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

#include "concretesite.h"
#include <QDomElement>

namespace LeechCraft
{
namespace DeadLyrics
{
	class MatcherBase
	{
	public:
		enum class Mode
		{
			Return,
			Exclude
		};
	protected:
		const Mode Mode_;

		MatcherBase (Mode mode)
		: Mode_ (mode)
		{
		}
	public:
		virtual ~MatcherBase () {}

		static std::shared_ptr<MatcherBase> MakeMatcher (Mode mode, const QDomElement& item);

		virtual QString Match (const QString&) const = 0;
	};

	class TagMatcher : public MatcherBase
	{
		const QString Tag_;
	public:
		TagMatcher (const QString& tag, Mode mode)
		: MatcherBase (mode)
		, Tag_ (tag)
		{
		}

		QString Match (const QString& str) const
		{
			// TODO
			return str;
		}
	};

	class RangeMatcher : public MatcherBase
	{
		const QString From_;
		const QString To_;
	public:
		RangeMatcher (const QString& from, const QString& to, Mode mode)
		: MatcherBase (mode)
		, From_ (from)
		, To_ (to)
		{
		}

		QString Match (const QString& string) const
		{
			int fromPos = string.indexOf (From_);
			const int toPos = string.indexOf (To_, fromPos + From_.size ());
			if (fromPos == -1 || toPos == -1)
				return Mode_ == Mode::Exclude ? string : QString ();

			if (Mode_ == Mode::Return)
			{
				fromPos += From_.size ();
				return string.mid (fromPos, toPos - fromPos);
			}
			else
				return string.left (fromPos) + string.right (toPos + To_.size ());
		}
	};

	MatcherBase_ptr MatcherBase::MakeMatcher (Mode mode, const QDomElement& item)
	{
		if (item.hasAttribute ("begin") && item.hasAttribute ("end"))
			return MatcherBase_ptr (new RangeMatcher (item.attribute ("from"), item.attribute ("to"), mode));
		else if (item.hasAttribute ("tag"))
			return MatcherBase_ptr (new TagMatcher (item.attribute ("tag"), mode));
		else
			return MatcherBase_ptr ();
	}

	ConcreteSite::ConcreteSite (const QDomElement& elem, QObject *parent)
	: QObject (parent)
	, Name_ (elem.attribute ("name"))
	, Charset_ (elem.attribute ("charset", "utf-8"))
	, URLTemplate_ (elem.attribute ("url"))
	{
		auto urlFormat = elem.firstChildElement ("urlFormat");
		while (!urlFormat.isNull ())
		{
			const auto& replace = urlFormat.attribute ("replace");
			const auto& with = urlFormat.attribute ("with");
			Q_FOREACH (const auto c, replace)
				Replacements_ [c] = with;

			urlFormat = urlFormat.nextSiblingElement ("urlFormat");
		}

		auto fillMatchers = [&elem] (const QString& name, MatcherBase::Mode mode)
		{
			QList<MatcherBase_ptr> result;

			auto extract = elem.firstChildElement (name);
			while (!extract.isNull ())
			{
				auto item = extract.firstChildElement ("item");
				while (!item.isNull ())
				{
					result << MatcherBase::MakeMatcher (mode, item);
					item = item.nextSiblingElement ("item");
				}

				extract = extract.nextSiblingElement (name);
			}
			result.removeAll (MatcherBase_ptr ());
			return result;
		};

		Extractors_ = fillMatchers ("extract", MatcherBase::Mode::Return);
		Excluders_ = fillMatchers ("exclude", MatcherBase::Mode::Exclude);
	}
}
}
