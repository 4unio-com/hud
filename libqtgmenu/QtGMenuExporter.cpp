#include "QtGMenuExporter.h"

#include <QMenu>

namespace qtgmenu
{

class QtGMenuExporterPrivate
{
};

QtGMenuExporter::QtGMenuExporter( const QString &objectPath, QMenu *menu,
    const QDBusConnection &_connection )
    : QObject( menu ),
      d( new QtGMenuExporterPrivate() )
{
}

QtGMenuExporter::~QtGMenuExporter()
{
}

} // namespace qtgmenu

#include "QtGMenuExporter.moc"
