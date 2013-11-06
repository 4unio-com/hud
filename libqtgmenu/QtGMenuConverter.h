#ifndef QTGMENUCONVERTER_H
#define QTGMENUCONVERTER_H

#include <memory>
#include <QMenu>

class _GMenuModel;
typedef _GMenuModel GMenuModel;

namespace qtgmenu
{

class QtGMenuConverterPrivate;

class QtGMenuConverter final
{
public:
  QtGMenuConverter();
  ~QtGMenuConverter();

  static std::shared_ptr< QMenu > ToQMenu( const GMenuModel& from_menu );
  static GMenuModel* ToGMenuModel( const QMenu& from_menu );

private:
  Q_DISABLE_COPY(QtGMenuConverter)
  std::unique_ptr< QtGMenuConverterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUCONVERTER_H
