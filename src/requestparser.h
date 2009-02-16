#ifndef REQUESTPARSER_H
#define REQUESTPARSER_H
#include <QObject>
#include <QStringList>

namespace LeechCraft
{
	struct Request
	{
		enum Type
		{
			RTFixed,
			RTWildcard,
			RTRegexp,
			RTTag
		};

		bool CaseSensitive_;
		Type Type_;
		QString Plugin_;
		QString Category_;
		QString String_;
		QStringList Params_;
	};

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

