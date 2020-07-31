/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_RPLISUPPORTPARSER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_RPLISUPPORTPARSER_H

#include <QObject>
#include <QMap>

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;

	class RplISupportParser : public QObject
	{
		Q_OBJECT
		IrcServerHandler *ISH_;
		QMap<QString, QString> ISupportMap_;
	public:
		RplISupportParser (IrcServerHandler*);
		bool ParseISupportReply (const QString&);
		QMap<QString, QString> GetISupportMap () const;
	private:
		void ConvertFromStdMapToQMap (const std::map<std::string, std::string>&);
	};
}
}
}
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_RPLISUPPORTPARSER_H
