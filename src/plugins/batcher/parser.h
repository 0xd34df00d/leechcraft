#ifndef PARSER_H
#define PARSER_H
#include <QObject>
#include <QStringList>

class Parser : public QObject
{
	Q_OBJECT
public:
	struct ParserData
	{
		QString Pattern_;
		int LowerBound_, UpperBound_, Step_;
		bool LeadingZeroes_;
	};

	Parser (QObject *parent = 0);
	QStringList Parse (const ParserData&) const;
};

#endif

