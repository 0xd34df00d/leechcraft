#ifndef SKINENGINE_H
#define SKINENGINE_H
#include <vector>
#include <QMap>
#include <QString>
#include <QList>
#include <QDir>

class QIcon;
class QAction;
class QTabWidget;

namespace LeechCraft
{
	class SkinEngine
	{
		QString OldIconSet_;
		typedef QMap<int, QString> sizef_t;
		QMap<QString, sizef_t> IconName2Path_;
		QMap<QString, QString> IconName2FileName_;
		QStringList IconSets_;

		SkinEngine ();
	public:
		static SkinEngine& Instance ();
		virtual ~SkinEngine ();

		QIcon GetIcon (const QString&, const QString&) const;
		void UpdateIconSet (const QList<QAction*>&);
		void UpdateIconSet (const QList<QTabWidget*>&);
		QStringList ListIcons () const;
	private:
		QString GetIconName (const QString&) const;
		void FindIconSets ();
		void FindIcons ();
		void FillMapping (const QString&, const QString&);
		void CollectDir (const QString&, const QString&);
		void CollectSubdir (QDir, const QString&, int);
		std::vector<int> GetDirForBase (const QString&, const QString&);
	};
};

#endif

