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
#include <stdexcept>
#include <QDomElement>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

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

		virtual QString operator() (const QString&) const = 0;
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

		QString operator() (const QString& string) const
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
				return string.left (fromPos) + string.mid (toPos + To_.size ());
		}
	};

	class TagMatcher : public MatcherBase
	{
		const QString Tag_;
		QString Name_;
	public:
		TagMatcher (const QString& tag, Mode mode)
		: MatcherBase (mode)
		, Tag_ (tag)
		{
			const int space = tag.indexOf (' ');
			if (space == -1)
				Name_ = tag;
			else
				Name_ = tag.left (space);
			Name_.remove ('<');
			Name_.remove ('>');
		}

		QString operator() (const QString& str) const
		{
			RangeMatcher rm (Tag_, "</" + Name_ + ">", Mode_);
			return rm (str);
		}
	};

	MatcherBase_ptr MatcherBase::MakeMatcher (Mode mode, const QDomElement& item)
	{
		if (item.hasAttribute ("begin") && item.hasAttribute ("end"))
			return MatcherBase_ptr (new RangeMatcher (item.attribute ("begin"), item.attribute ("end"), mode));
		else if (item.hasAttribute ("tag"))
			return MatcherBase_ptr (new TagMatcher (item.attribute ("tag"), mode));
		else
			return MatcherBase_ptr ();
	}

	ConcreteSiteDesc::ConcreteSiteDesc (const QDomElement& elem)
	: Name_ (elem.attribute ("name"))
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

		Matchers_ += fillMatchers ("extract", MatcherBase::Mode::Return);
		Matchers_ += fillMatchers ("exclude", MatcherBase::Mode::Exclude);
	}

	ConcreteSite::ConcreteSite (const Media::LyricsQuery& query,
			const ConcreteSiteDesc& desc, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Query_ (query)
	, Desc_ (desc)
	{
		auto replace = [this] (QString str)
		{
			Q_FOREACH (const QChar c, Desc_.Replacements_.keys ())
				str.replace (c, Desc_.Replacements_ [c]);
			return str;
		};

		const auto& artist = replace (query.Artist_.toLower ());
		const auto& album = replace (query.Album_.toLower ());
		const auto& title = replace (query.Title_.toLower ());

		auto url = Desc_.URLTemplate_;
		url.replace ("{artist}", artist);
		url.replace ("{album}", album);
		url.replace ("{title}", title);

		auto cap = [] (QString str)
		{
			if (!str.isEmpty ())
				str [0] = str [0].toUpper ();
			return str;
		};
		url.replace ("{Artist}", cap (artist));
		url.replace ("{Album}", cap (album));
		url.replace ("{Title}", cap (title));

		auto nam = proxy->GetNetworkAccessManager ();
		auto reply = nam->get (QNetworkRequest (QUrl (url)));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (deleteLater ()));
	}

	namespace
	{
		void FixEntities (QString& str)
		{
			int pos = 0;
			while ((pos = str.indexOf ("&#", pos)) != -1)
			{
				const int ePos = str.indexOf (';', pos + 2);
				if (ePos == -1 || ePos - pos > 5)
				{
					pos = ePos;
					continue;
				}

				const char val = static_cast<char> (str.mid (pos + 2, ePos - pos - 2).toShort ());
				if (val > 0)
				{
					str.replace (pos, ePos - pos + 1, QChar (val));
					++pos;
				}
			}
		}
	}

	void ConcreteSite::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		deleteLater ();

		const auto& data = reply->readAll ();
		auto str = QString::fromUtf8 (data.constData ());
		FixEntities (str);

		Q_FOREACH (auto excluder, Desc_.Matchers_)
			str = (*excluder) (str);

		emit gotLyrics (Query_, QStringList (str.trimmed ()));
	}
}
}
