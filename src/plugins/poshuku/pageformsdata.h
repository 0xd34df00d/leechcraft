#ifndef PLUGINS_POSHUKU_PAGEFORMSDATA_H
#define PLUGINS_POSHUKU_PAGEFORMSDATA_H
#include <QMap>
#include <QString>
#include <QVariant>

class QDebug;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			struct ElementData
			{
				int FormIndex_;
				QString Name_;
				QString Type_;
				QVariant Value_;
			};

			QDebug& operator<< (QDebug&, const ElementData&);

			/** Holds information about all the elements on the single form.
			 */
			typedef QList<ElementData> ElementsData_t;

			/** Holds information about all the forms/pages, identified by their
			 * URL.
			 */
			typedef QMap<QString, ElementsData_t> PageFormsData_t;

			struct ElemFinder
			{
				const QString& ElemName_;
				const QString& ElemType_;

				ElemFinder (const QString& en, const QString& et)
				: ElemName_ (en)
				, ElemType_ (et)
				{
				}

				inline bool operator() (const ElementData& ed) const
				{
					return ed.Name_ == ElemName_ &&
						ed.Type_ == ElemType_;
				}
			};
		};
	};
};

#endif

