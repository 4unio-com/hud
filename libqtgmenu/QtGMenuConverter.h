#ifndef QTGMENUCONVERTER_H
#define QTGMENUCONVERTER_H

#include <memory>
#include <QtCore>

namespace qtgmenu
{

class QtGMenuConverterPrivate;

class QtGMenuConverter final
{
public:
  QtGMenuConverter();
  ~QtGMenuConverter();

private:
  Q_DISABLE_COPY(QtGMenuConverter)
  std::unique_ptr< QtGMenuConverterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUCONVERTER_H
