#include <QtGMenuImporter.h>
#include <internal/QtGMenuImporterPrivate.h>

#include <QIcon>
#include <QMenu>

namespace qtgmenu
{

QtGMenuImporter::QtGMenuImporter( const QString &service, const QString &path, QObject *parent )
    : QObject( parent ),
      d( new QtGMenuImporterPrivate( service, path, *this ) )
{
}

QtGMenuImporter::~QtGMenuImporter()
{
}

std::shared_ptr< QMenu > QtGMenuImporter::Menu() const
{
  return d->GetQMenu();
}

int QtGMenuImporter::GetItemCount()
{
  GMenu* menu = d->GetGMenu();
  return !menu ? 0 : g_menu_model_get_n_items( G_MENU_MODEL( menu ) );
}

} // namespace qtgmenu

#include <QtGMenuImporter.moc>
