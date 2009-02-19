#include "requestparser.h"

using namespace LeechCraft;

RequestParser::RequestParser (const QString& string, QObject *parent)
: QObject (parent)
{
	Parse (string);
}

void RequestParser::Parse (const QString& request)
{
	Request_ = Request ();
	Request_.CaseSensitive_ = false;

	QStringList tokens = request.split (' ', QString::SkipEmptyParts);
	foreach (QString token, tokens)
	{
		int index = token.indexOf (':');
		// Skip if there is no :, if it is first or last.
		if (index == -1 ||
				index == 0 ||
				token.size () - 1 == index)
			continue;

		// If : is escaped, erase the \ symbol and skip.
		if (token [index - 1] == '\\')
		{
			token.remove (index - 1, 1);
			continue;
		}

		tokens.removeAll (token + ' ');
		// This is for the case where the token is the last token in the
		// request, so there is no token + ' '
		tokens.removeAll (token);

		QString key = token.left (index).toLower ();
		QString value = token.mid (index + 1);

		if (key == tr ("pl") ||
				key == tr ("plugin"))
			Request_.Plugin_ = value;
		else if (key == tr ("ca") ||
				key == tr ("category"))
			Request_.Category_ = value;
		else if (key == tr ("cs") ||
				key == tr ("casesensitive"))
			Request_.CaseSensitive_ = (value.toLower () == tr ("true") ||
					value.toLower () == tr ("on"));
		else if (key == tr ("t") ||
				key == tr ("type"))
		{
			if (value == tr ("f") ||
					value == tr ("fixed"))
				Request_.Type_ = Request::RTFixed;
			else if (value == tr ("w") ||
					value == tr ("wildcard"))
				Request_.Type_ = Request::RTWildcard;
			else if (value == tr ("r") ||
					value == tr ("regexp"))
				Request_.Type_ = Request::RTRegexp;
			else if (value == tr ("t") ||
					value == tr ("tag"))
				Request_.Type_ = Request::RTTag;
		}
		else if (key == tr ("pa") ||
				key == tr ("param"))
			Request_.Params_ << value;
	}
	Request_.String_ = tokens.join (" ");
}

const Request& RequestParser::GetRequest () const
{
	return Request_;
}

