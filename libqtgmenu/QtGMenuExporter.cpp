#include "QtGMenuExporter.h"

#include <QMenu>

using namespace qtgmenu;

namespace qtgmenu
{

class QtGMenuExporterPrivate
{
};

} // namespace qtgmenu

QtGMenuExporter::QtGMenuExporter( const QString& dbusObjectPath, QMenu* menu,
    const QDBusConnection& connection )
    : QObject( menu ),
      d( new QtGMenuExporterPrivate() )
{
}

QtGMenuExporter::~QtGMenuExporter()
{
}

#include "QtGMenuExporter.moc"
