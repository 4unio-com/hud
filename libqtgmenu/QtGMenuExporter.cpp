#include "QtGMenuExporter.h"

#include <QMenu>

namespace qtgmenu
{

class QtGMenuExporterPrivate
{
};

QtGMenuExporter::QtGMenuExporter( const QString& dbusObjectPath, QMenu* menu,
    const QDBusConnection& connection )
    : QObject( menu ),
      d( new QtGMenuExporterPrivate() )
{
}

QtGMenuExporter::~QtGMenuExporter()
{
}

} // namespace qtgmenu

#include "QtGMenuExporter.moc"
