#include <QtDebug>
#include "parser.h"

Parser::Parser (QObject *parent)
: QObject (parent)
{
}

QStringList Parser::Parse (const Parser::ParserData& pd) const
{
	QStringList result;
	QString pattern = pd.Pattern_;
	if (!pattern.size ())
		return QStringList ();

	int dollar = pattern.indexOf ('$');

	if (dollar == -1)
		return QStringList (pattern);

	int width = 0;
	if (pd.LeadingZeroes_)
		width = QString::number (pd.UpperBound_).size ();
	for (int i = pd.LowerBound_; i <= pd.UpperBound_; i += pd.Step_)
		result << QString (pattern).replace (dollar, 1, QString ("%1").arg (i, width, 10, QChar ('0')));

	return result;
}

