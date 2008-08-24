#ifndef XMLSETTINGSMANAGER_H
#define XMLSETTINGSMANAGER_H
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace Main
{
	class XmlSettingsManager : public BaseSettingsManager
	{
		Q_OBJECT

		XmlSettingsManager ();
	public:
		virtual ~XmlSettingsManager ();
		static XmlSettingsManager* Instance ();
	protected:
		virtual QSettings* BeginSettings () const;
		virtual void EndSettings (QSettings*) const;
	};
};

#endif

