#include <QtDebug>
#include <QByteArray>
#include "bencodeparser.h"

bool BencodeParser::Parse (const QByteArray& code)
{
	if (code.isEmpty () || code.isNull ())
	{
		Error_ = QObject::tr ("Bencoded data is empty.");
		return false;
	}
	Code_ = code;
	Index_ = 0;
	InfoStart_ = 0;
	InfoLength_ = 0;

	return GetDictionary (Parsed_);
}

QString BencodeParser::GetErrorString () const
{
	return Error_;
}

const Dictionary_t& BencodeParser::GetParsed () const
{
	return Parsed_;
}

QByteArray BencodeParser::GetInfoSection () const
{
	return Code_.mid (InfoStart_, InfoLength_);
}

bool BencodeParser::GetString (QByteArray& str)
{
	const int totalSize = Code_.size ();
	int strSize = -1;

	do
	{
		QChar c = Code_.at (Index_);
		if (c < '0' || c > '9')
		{
			if (strSize == -1)
				return false;
			if (c != ':')
			{
				Error_ = QObject::tr ("Unexpected character at %1: %2").arg (Index_).arg (c);
				return false;
			}
			++Index_;
			break;
		}

		if (strSize == -1)
			strSize = 0;
		strSize *= 10;
		strSize += c.toAscii () - '0';
	} while (++Index_ < totalSize);

	str = Code_.mid (Index_, strSize);
	Index_ += strSize;
	return true;
}

bool BencodeParser::GetInteger (qint64& integer)
{
	if (Code_.at (Index_) != 'i')
		return false;

	const int totalSize = Code_.size ();

	++Index_;

	qint64 curNum = -1;
	bool neg = false;

	do
	{
		QChar c = Code_.at (Index_);
		if (c < '0' || c > '9')
		{
			if (curNum == -1)
			{
				if (c != '-' || neg)
					return false;
				neg = true;
				continue;
			}
			else
			{
				if (c != 'e')
				{
					Error_ = QObject::tr ("Unexpected character at %1: %2").arg (Index_).arg (c);
					return false;
				}
				++Index_;
				break;
			}
		}

		if (curNum == -1)
			curNum = 0;
		curNum *= 10;
		curNum += c.toAscii () - '0';
	} while (++Index_ < totalSize);

	if (neg)
		curNum *= -1;

	integer = curNum;
	return true;
}

bool BencodeParser::GetList (QList<QVariant>& list)
{
	if (Code_.at (Index_) != 'l')
		return false;

	const int totalSize = Code_.size ();
	
	QList<QVariant> result;
	++Index_;

	do
	{
		if (Code_.at (Index_) == 'e')
		{
			++Index_;
			break;
		}

		qint64 num;
		QByteArray str;
		QList<QVariant> lst;
		Dictionary_t dict;

		if (GetInteger (num))
			result << num;
		else if (GetString (str))
			result << str;
		else if (GetList (lst))
			result << lst;
		else if (GetDictionary (dict))
			result << qVariantFromValue<Dictionary_t> (dict);
		else
		{
			Error_ = QObject::tr ("Listparser: Err@index %1, bencoded bytearray is malformed.").arg (Index_);
			return false;
		}
	} while (Index_ < totalSize);

	list = result;
	return true;
}

bool BencodeParser::GetDictionary (Dictionary_t& dict)
{
	if (Code_.at (Index_) != 'd')
		return false;

	const int totalSize = Code_.size ();

	Dictionary_t result;
	++Index_;

	do
	{
		if (Code_.at (Index_) == 'e')
		{
			++Index_;
			break;
		}

		QByteArray key;
		if (!GetString (key))
			break;

		if (key == "info")
			InfoStart_ = Index_;

		qint64 num;
		QByteArray str;
		QList<QVariant> lst;
		Dictionary_t dict;

		if (GetInteger (num))
			result.insert (key, num);
		else if (GetString (str))
			result.insert (key, str);
		else if (GetList (lst))
			result.insert (key, lst);
		else if (GetDictionary (dict))
			result.insert (key, qVariantFromValue<Dictionary_t> (dict));
		else
		{
			Error_ = QObject::tr ("Dictparser: Err@index %1, bencoded bytearray is malformed.").arg (Index_);
			return false;
		}

		if (key == "info")
			InfoLength_ = Index_ - InfoStart_;
	} while (Index_ < totalSize);

	dict = result;
	return true;
}

