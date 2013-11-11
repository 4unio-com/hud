#ifndef QTGMENUUTILS_H
#define QTGMENUUTILS_H

#include <QtCore>

class _GVariant;
typedef _GVariant GVariant;

namespace qtgmenu
{

class QtGMenuUtils final
{
public:
  static QVariant GVariantToQVariant( GVariant* gvariant );
};

} // namespace qtgmenu

#endif // QTGMENUUTILS_H
