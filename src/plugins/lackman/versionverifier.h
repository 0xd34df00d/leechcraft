#ifndef PLUGINS_LACKMAN_VERSIONVERIFIER_H
#define PLUGINS_LACKMAN_VERSIONVERIFIER_H

#include <QMap>
#include <QRegExp>

class VersionVerifier
{
public:
	VersionVerifier() : m_verRegexp("^(cvs\\.)?(\\d+)((\\.\\d+)*)([a-z]?)((_(pre|p|beta|alpha|rc)\\d*)*)(-r('\\d+'))?$"),
						m_suffixRegexp("^(alpha|beta|rc|pre|p)(\\d*)$")
	{
	}

	int VerifyVersion(const QString& version) const
	{
		if (m_verRegexp.exactMatch(version))
		{
			return 1;
		}
		else
		{
			//~ print(_("!!! syntax error in version: %s") % myver)
			return 0;
		}
	}

	int CompareVersions(const QString& version1, const QString& version2) const
	{
		if (version1 == version2)
		{
			return 0;
		}

		QString key = version1 + ":" +version2;
		CacheMap::iterator cachedVal = m_cacheMap.find(key);
		if (cachedVal != m_cacheMap.end())
		{
			return cachedVal.value ();
		}
		if ((!VerifyVersion(version1))||(!VerifyVersion(version2)))
		{
			return 0; // throw smth?
		}
		int pos1 = m_verRegexp.indexIn(version1);
		QStringList match1 = m_verRegexp.capturedTexts();
		int pos2 = m_verRegexp.indexIn(version2);
		QStringList match2 = m_verRegexp.capturedTexts();

		// we can shrink it with != , but readability would suffer in that case
		if ((!match1[0].isEmpty()) && (match2[0].isEmpty()))
		{
			m_cacheMap[key] = 1;
			return 1;
		}
		else if ((!match2[0].isEmpty()) && (match1[0].isEmpty()))
		{
			m_cacheMap[key] = -1;
			return -1;
		}

		// building lists of the version parts before the suffix
		// first part is simple
		QList<int> list1, list2;
		list1.push_back(match1[1].toInt());
		list2.push_back(match2[1].toInt());

		// this part would greatly benefit from a fixed-length version pattern
		if ((!match1[2].isEmpty())||(!match2[2].isEmpty()))
		{
			QStringList vlist1 = match1[2].mid(1).split(".");
			QStringList vlist2 = match2[2].mid(1).split(".");
			int maxLen = std::max(vlist1.count(),vlist2.count());
			for (int i=0;i<maxLen; ++i)
			{
				// Implcit .0 is given a value of -1, so that 1.0.0 > 1.0, since it
				// would be ambiguous if two versions that aren't literally equal
				// are given the same value (in sorting, for example).
				if ((vlist1.count() <= i) || (vlist1[i].isEmpty()))
				{
					list1.push_back(-1);
					list2.push_back(vlist2[i].toInt());
				}
				else if ((vlist2.count() <= i) || (vlist2[i].isEmpty()))
				{
					list1.push_back(vlist1[i].toInt());
					list2.push_back(-1);
				}

				// Let's make life easy and use integers unless we're forced to use floats
				else if ((vlist1[i][0] != '0') && (vlist2[i][0] != '0'))
				{
					list1.push_back(vlist1[i].toInt());
					list2.push_back(vlist2[i].toInt());
				}
				// now we have to use floats so 1.02 compares correctly against 1.1
				else
				{
					// wtf???? after int(ljust(x,0)) is the same as int
					//list1.push_back(int(vlist1[i].ljust(max_len, "0")))
					//list2.push_back(int(vlist2[i].ljust(max_len, "0")))
					list1.push_back(vlist1[i].toInt());
					list2.push_back(vlist2[i].toInt());
				}
			}
		}
		// and now the final letter
		// NOTE: Behavior changed in r2309 (between portage-2.0.x and portage-2.1).
		// The new behavior is 12.2.5 > 12.2b which, depending on how you look at,
		// may seem counter-intuitive. However, if you really think about it, it
		// seems like it's probably safe to assume that this is the behavior that
		// is intended by anyone who would use versions such as these.
		if (!match1[4].isEmpty())
		{
			// CHECK if this is really
			list1.push_back(match1[4][0].toAscii ()); // push char
		}
		if (!match2[4].isEmpty())
		{
			list2.push_back(match2[4][0].toAscii ()); // push char
		}

		for (int i =0; i<std::max(list1.count(),list2.count());++i)
		{
			if (list1.count() <= i)
			{
				m_cacheMap[key] = -1;
				return -1;
			}
			else if (list2.count() <= i)
			{
				m_cacheMap[key] = 1;
				return 1;
			}
			else if (list1[i] != list2[i])
			{

				int a = list1[i];
				int b = list2[i];
				int rval = a > b ? 1 : (a<b?-1:0);
				m_cacheMap[key] = rval;
				return rval;
			}
		}

		// main version is equal, so now compare the _suffix part
		QStringList slist1 = match1[5].split("_");
		QStringList slist2 = match2[5].split("_");

		QStringList s1, s2;
		// start with 1
		for (int i=1;i<std::max(slist1.count(),slist2.count());++i)
		{
			// Implicit _p0 is given a value of -1, so that 1 < 1_p0
			if (slist1.count() <= i)
			{
				s1.push_back("p");
				s1.push_back("-1");
			}
			else
			{
				int pos = m_suffixRegexp.indexIn(slist1[i]);
				s1 = m_suffixRegexp.capturedTexts();
			}
			if (slist2.count() <= i)
			{
				s2.push_back("p");
				s2.push_back("-1");
			}
			else
			{
				int pos = m_suffixRegexp.indexIn(slist2[i]);
				s2 = m_suffixRegexp.capturedTexts();
			}

			// following map should be static. I'm too lazy to make it static :)
			std::map<QString, int> suffixValueMap;
			suffixValueMap["pre"] = -2;
			suffixValueMap["p"] = 0;
			suffixValueMap["alpha"] = -4;
			suffixValueMap["beta"] = -3;
			suffixValueMap["rc"] = -1;

			if (s1[0] != s2[0])
			{
				int a = suffixValueMap[s1[0]];
				int b = suffixValueMap[s2[0]];
				int rval = a > b ? 1 : (a<b?-1:0);
				m_cacheMap[key] = rval;
				return rval;
			}
			if (s1[1] != s2[1])
			{
				//it's possible that the s(1|2)[1] == ''
				// in such a case, fudge it.
				bool fl = false;
				int r1 = s1[1].toInt(&fl);
				if (!fl)
				{
					r1 = 0;
				}
				int r2 = s2[1].toInt(&fl);
				if (!fl)
				{
					r2 = 0;
				}

				int rval = r1 > r2 ? 1 : (r1<r2?-1:0);
				if (rval)
				{
					m_cacheMap[key] = rval;
					return rval;
				}
			}
		}
		// the suffix part is equal to, so finally check the revision
		int r1 = 0;
		int r2 = 0;
		if (!match1[9].isEmpty())
		{
			r1 = match1[9].toInt();
		}
		if (!match2[9].isEmpty())
		{
			r2 = match2[9].toInt();
		}

		int rval = r1 > r2 ? 1 : (r1<r2?-1:0);
		m_cacheMap[key] = rval;
		return rval;
	}

	private:
		typedef QMap<QString, int> CacheMap;

		mutable CacheMap	 	m_cacheMap;
		QRegExp					m_verRegexp;
		QRegExp					m_suffixRegexp;
	};

#endif
