#ifndef QTGMENUCONVERTER_H
#define QTGMENUCONVERTER_H

#include <memory>
#include <QMenu>

class _GMenu;
typedef _GMenu GMenu;

namespace qtgmenu
{

class QtGMenuConverterPrivate;

class QtGMenuConverter final
{
public:
  QtGMenuConverter();
  ~QtGMenuConverter();

  static std::shared_ptr< QMenu > ToQMenu( const GMenu& from_menu );
  static GMenu* ToGMenu( const QMenu& from_menu );

private:
  Q_DISABLE_COPY(QtGMenuConverter)
  std::unique_ptr< QtGMenuConverterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUCONVERTER_H
