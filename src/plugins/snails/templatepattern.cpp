/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "templatepattern.h"
#include <QHash>
#include <util/sll/prelude.h>
#include <interfaces/itexteditor.h>
#include "account.h"
#include "util.h"
#include "messagebodies.h"

namespace LC
{
namespace Snails
{
	namespace
	{
		template<typename F>
		PatternFunction_t Wrap (const F& f, std::result_of_t<F (MessageInfo)>* = nullptr)
		{
			return [f] (const Account*, const MessageInfo& info, const MessageBodies&, ContentType)
			{
				return f (info);
			};
		}

		template<typename F>
		PatternFunction_t Wrap (const F& f, std::result_of_t<F (MessageInfo, MessageBodies)>* = nullptr)
		{
			return [f] (const Account*, const MessageInfo& info, const MessageBodies& bodies, ContentType)
					{
						return f (info, bodies);
					};
		}

		template<typename F>
		PatternFunction_t Wrap (const F& f, std::result_of_t<F (MessageInfo, ContentType)>* = nullptr)
		{
			return [f] (const Account*, const MessageInfo& info, const MessageBodies&, ContentType ct)
			{
				return f (info, ct);
			};
		}

		template<typename F>
		PatternFunction_t Wrap (const F& f, std::result_of_t<F (MessageInfo, MessageBodies, ContentType)>* = nullptr)
		{
			return [f] (const Account*, const MessageInfo& info, const MessageBodies& bodies, ContentType ct)
					{
						return f (info, bodies, ct);
					};
		}

		QString FormatNameAndEmail (ContentType ct, const Address& addr)
		{
			const auto& result = (addr.Name_.isEmpty () ? "" : addr.Name_ + " ") + "<" + addr.Email_ + ">";
			return ct == ContentType::HTML ? result.toHtmlEscaped () : result;
		}

		QString MakePlainQuote (const QString& plainText)
		{
			auto plainSplit = plainText.split ('\n');
			for (auto& str : plainSplit)
			{
				str = str.trimmed ();
				if (!str.isEmpty () && str.at (0) != '>')
					str.prepend (' ');
				str.prepend ('>');
			}

			return plainSplit.join ("\n") + "\n\n";
		}

		QString MakeQuote (ContentType ct, const MessageBodies& bodies)
		{
			switch (ct)
			{
			case ContentType::PlainText:
				return MakePlainQuote (bodies.PlainText_);
			case ContentType::HTML:
				return !bodies.HTML_.isEmpty () ?
						bodies.HTML_ :
						PlainBody2HTML (bodies.PlainText_);
			}
		}
	}

	QList<TemplatePattern> GetKnownPatterns ()
	{
		static const QLocale loc;
		static const QList<TemplatePattern> patterns
		{
			{
				"ODATE",
				Wrap ([] (const MessageInfo& info) { return loc.toString (info.Date_.date (), QLocale::ShortFormat); })
			},
			{
				"OTIME",
				Wrap ([] (const MessageInfo& info) { return loc.toString (info.Date_.time (), QLocale::ShortFormat); })
			},
			{
				"ODATETIME",
				Wrap ([] (const MessageInfo& info) { return loc.toString (info.Date_, QLocale::ShortFormat); })
			},
			{
				"ONAME",
				Wrap ([] (const MessageInfo& info) { return info.Addresses_ [AddressType::From].value (0).Name_; })
			},
			{
				"OEMAIL",
				Wrap ([] (const MessageInfo& info) { return info.Addresses_ [AddressType::From].value (0).Email_; })
			},
			{
				"OSUBJECT",
				Wrap ([] (const MessageInfo& info) { return info.Subject_; })
			},
			{
				"ONAMEOREMAIL",
				Wrap ([] (const MessageInfo& info)
						{
							const auto& addr = info.Addresses_ [AddressType::From].value (0);
							return addr.Name_.isEmpty () ? "<" + addr.Email_ + ">" : addr.Name_;
						})
			},
			{
				"ONAMEANDEMAIL",
				Wrap ([] (const MessageInfo& info, ContentType ct)
						{
							return FormatNameAndEmail (ct, info.Addresses_ [AddressType::From].value (0));
						})
			},
			{
				"TONAMEANDEMAIL",
				Wrap ([] (const MessageInfo& info, ContentType ct)
						{
							return Util::Map (info.Addresses_ [AddressType::To],
									Util::Curry (&FormatNameAndEmail) (ct))
								.join ("; ");
						})
			},
			{
				"CCNAMEANDEMAIL",
				Wrap ([] (const MessageInfo& info, ContentType ct)
						{
							return Util::Map (info.Addresses_ [AddressType::Cc],
									Util::Curry (&FormatNameAndEmail) (ct))
								.join ("; ");
						})
			},
			{
				"OBODY",
				[] (const Account*, const MessageInfo&, const MessageBodies& bodies, ContentType type)
				{
					switch (type)
					{
					case ContentType::PlainText:
						return bodies.PlainText_;
					case ContentType::HTML:
						return !bodies.HTML_.isEmpty () ?
								bodies.HTML_ :
								PlainBody2HTML (bodies.PlainText_);
					}
				}
			},
			{
				"QUOTE",
				[] (const Account*, const MessageInfo&, const MessageBodies& bodies, ContentType ct)
				{
					return MakeQuote (ct, bodies);
				}
			},
			{
				"SIGNATURE",
				[] (const Account *acc, const MessageInfo&, const MessageBodies&, ContentType type)
				{
					const auto& userName = acc->GetConfig ().UserName_;
					switch (type)
					{
					case ContentType::PlainText:
						return "-- \n  " + userName + "\n";
					case ContentType::HTML:
						return "--&nbsp;<br/>&nbsp;" + userName + "<br/>";
					}
				}
			},
			{
				"CURSOR",
				[] (const Account*, const MessageInfo&, const MessageBodies&, ContentType type)
				{
					switch (type)
					{
					case ContentType::PlainText:
						return "${CURSOR}";
					case ContentType::HTML:
						return "<div id='place_cursor_here'>&nbsp;</div>";
					}
				}
			},
		};

		return patterns;
	}

	QHash<QString, PatternFunction_t> GetKnownPatternsHash ()
	{
		static const auto hash = []
				{
					QHash<QString, PatternFunction_t> result;
					for (const auto& pattern : GetKnownPatterns ())
						result [pattern.PatternText_] = pattern.Substitute_;
					return result;
				} ();
		return hash;
	}
}
}
