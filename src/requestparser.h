#ifndef REQUESTPARSER_H
#define REQUESTPARSER_H
#include <QObject>
#include <QStringList>
#include <interfaces/ifinder.h>

namespace LeechCraft
{
	class RequestParser : public QObject
	{
		Q_OBJECT

		Request Request_;
	public:
		RequestParser (const QString& = QString (), QObject* = 0);

		void Parse (const QString&);
		const Request& GetRequest () const;
	};
};

#endif

