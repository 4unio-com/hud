#include <QtGMenuImporter.h>
#include <internal/QtGMenuImporterPrivate.h>

#include <QIcon>
#include <QMenu>

namespace qtgmenu
{

QtGMenuImporter::QtGMenuImporter(const QString& service, const QString& path, QObject* parent)
    : QObject(parent),
      d(new QtGMenuImporterPrivate( service, path ))
{
}

QtGMenuImporter::~QtGMenuImporter()
{
}

std::shared_ptr<QMenu> QtGMenuImporter::Menu() const
{
    GMenuModel* model = d->GetGMenuModel();
    if ( model == nullptr )
    {
        return nullptr;
    }

    return d->GetQMenu();
}

} // namespace qtgmenu

#include "QtGMenuImporter.moc"
