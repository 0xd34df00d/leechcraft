#include "morphfile.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			MorphFile::MorphFile (const QString& name)
			: QFile (name)
			, Gunzip_ (false)
			, Counter_ (0)
			{
			}
			
			MorphFile::MorphFile (QObject *parent)
			: QFile (parent)
			, Gunzip_ (false)
			, Counter_ (0)
			{
			}
			
			MorphFile::MorphFile (const QString& name, QObject *parent)
			: QFile (name, parent)
			, Gunzip_ (false)
			, Counter_ (0)
			{
			}
			
			MorphFile::~MorphFile ()
			{
			}
			
			void MorphFile::AddRef ()
			{
				++Counter_;
			}
			
			void MorphFile::Release ()
			{
				--Counter_;
				if (!Counter_)
					deleteLater ();
			}
			
			void MorphFile::Gunzip (bool state)
			{
				Gunzip_ = state;
			}
			
			void intrusive_ptr_add_ref (MorphFile *file)
			{
				file->AddRef ();
			}
			
			void intrusive_ptr_release (MorphFile *file)
			{
				file->Release ();
			}
		};
	};
};

