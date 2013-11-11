#include "QtGMenuConverter.h"
#include <internal/QtGMenuImporterPrivate.h>

#undef signals
#include <gio/gio.h>

using namespace qtgmenu;

namespace qtgmenu
{

class QtGMenuConverterPrivate
{
};

} // namespace qtgmenu

QtGMenuConverter::QtGMenuConverter()
    : d( new QtGMenuConverterPrivate() )
{
}

QtGMenuConverter::~QtGMenuConverter()
{
}

std::shared_ptr< QMenu > QtGMenuConverter::ToQMenu( const GMenuModel& from_menu )
{
  return std::shared_ptr < QMenu > ( new QMenu() );
}

GMenuModel* QtGMenuConverter::ToGMenuModel( const QMenu& from_menu )
{
  return G_MENU_MODEL( g_menu_new() ) ;
}
