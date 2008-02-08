#include "datatypes.h"

namespace
{
   const int IntRangeVersion = 19;
};

QDataStream& operator<< (QDataStream& out, const IntRange& val)
{
   out << IntRangeVersion << val.first << val.second;
   return out;
}

QDataStream& operator>> (QDataStream& in, IntRange& val)
{
   int version;
   in >> version;
   if (version == IntRangeVersion)
      in >> val.first >> val.second;
   return in;
}

