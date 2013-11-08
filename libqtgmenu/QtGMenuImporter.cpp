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

void QtGMenuImporter::ForceRefresh()
{
  d->StartPollTimer( 100 );
}

int QtGMenuImporter::GetItemCount()
{
  GMenuModel* model = d->GetGMenuModel();

  if( !model )
  {
    return 0;
  }

  gint item_count = g_menu_model_get_n_items( model );
  g_object_unref( model );

  return item_count;
}

} // namespace qtgmenu

#include <QtGMenuImporter.moc>
