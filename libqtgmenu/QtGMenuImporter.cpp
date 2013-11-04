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
    delete d;
}

std::shared_ptr< QMenu > QtGMenuImporter::menu() const
{
    GMenuModel* model = d->GetGMenuModel();
    if( model == nullptr )
    {
        return nullptr;
    }

    g_object_unref(model);
    return d->GetQMenu();
}

void QtGMenuImporter::updateMenu()
{
}

} // namespace qtgmenu

#include "QtGMenuImporter.moc"
