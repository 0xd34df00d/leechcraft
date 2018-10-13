/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "templatepattern.h"
#include <QHash>
#include <util/sll/prelude.h>
#include <util/sll/curry.h>
#include <interfaces/itexteditor.h>
#include "message.h"
#include "account.h"
#include "util.h"

namespace LeechCraft
{
namespace Snails
{
	namespace
	{
		template<typename F>
		PatternFunction_t Wrap (const F& f, std::result_of_t<F (const Message*)>* = nullptr)
		{
			return [f] (const Account*, const Message *msg, ContentType, const QString&)
					{
						if (!msg)
							return QString {};

						return f (msg);
					};
		}

		template<typename F>
		PatternFunction_t Wrap (const F& f, std::result_of_t<F (const Message*, ContentType)>* = nullptr)
		{
			return [f] (const Account*, const Message *msg, ContentType ct, const QString&)
					{
						if (!msg)
							return QString {};

						return f (msg, ct);
					};
		}

		QString FormatNameAndEmail (ContentType ct, const Address_t& addr)
		{
			const auto& result = (addr.first.isEmpty () ? "" : addr.first + " ") + "<" + addr.second + ">";
			return ct == ContentType::HTML ? result.toHtmlEscaped () : result;
		}
	}

	QList<TemplatePattern> GetKnownPatterns ()
	{
		static const QList<TemplatePattern> patterns
		{
			{
				"ODATE",
				Wrap ([] (const Message *msg) { return msg->GetDate ().date ().toString (Qt::DefaultLocaleShortDate); })
			},
			{
				"OTIME",
				Wrap ([] (const Message *msg) { return msg->GetDate ().time ().toString (Qt::DefaultLocaleShortDate); })
			},
			{
				"ODATETIME",
				Wrap ([] (const Message *msg) { return msg->GetDate ().toString (Qt::DefaultLocaleShortDate); })
			},
			{
				"ONAME",
				Wrap ([] (const Message *msg) { return msg->GetAddress (AddressType::From).first; })
			},
			{
				"OEMAIL",
				Wrap ([] (const Message *msg) { return msg->GetAddress (AddressType::From).second; })
			},
			{
				"OSUBJECT",
				Wrap ([] (const Message *msg) { return msg->GetSubject (); })
			},
			{
				"ONAMEOREMAIL",
				Wrap ([] (const Message *msg)
						{
							const auto& addr = msg->GetAddress (AddressType::From);
							return addr.first.isEmpty () ? "<" + addr.second + ">" : addr.first;
						})
			},
			{
				"ONAMEANDEMAIL",
				Wrap ([] (const Message *msg, ContentType ct)
						{
							return FormatNameAndEmail (ct, msg->GetAddress (AddressType::From));
						})
			},
			{
				"TONAMEANDEMAIL",
				Wrap ([] (const Message *msg, ContentType ct)
						{
							return Util::Map (msg->GetAddresses (AddressType::To),
									Util::Curry (&FormatNameAndEmail) (ct))
								.join ("; ");
						})
			},
			{
				"CCNAMEANDEMAIL",
				Wrap ([] (const Message *msg, ContentType ct)
						{
							return Util::Map (msg->GetAddresses (AddressType::Cc),
									Util::Curry (&FormatNameAndEmail) (ct))
								.join ("; ");
						})
			},
			{
				"OBODY",
				Wrap ([] (const Message *msg, ContentType type)
						{
							switch (type)
							{
							case ContentType::PlainText:
								return msg->GetBody ();
							case ContentType::HTML:
							{
								const auto& html = msg->GetHTMLBody ();
								return !html.isEmpty () ?
										html :
										PlainBody2HTML (msg->GetBody ());
							}
							}
						})
			},
			{
				"QUOTE",
				[] (const Account*, const Message*, ContentType, const QString& body) { return body; }
			},
			{
				"SIGNATURE",
				[] (const Account *acc, const Message*, ContentType type, const QString&)
				{
					switch (type)
					{
					case ContentType::PlainText:
						return "-- \n  " + acc->GetUserName () + "\n";
					case ContentType::HTML:
						return "--&nbsp;<br/>&nbsp;" + acc->GetUserName () + "<br/>";
					}
				}
			},
			{
				"CURSOR",
				[] (const Account*, const Message*, ContentType type, const QString&)
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
