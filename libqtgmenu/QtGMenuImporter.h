#ifndef QTGMENUIMPORTER_H
#define QTGMENUIMPORTER_H

#include <QObject>
#include <memory>

class QAction;
class QDBusPendingCallWatcher;
class QIcon;
class QMenu;

class _GMenu;
typedef _GMenu GMenu;

namespace qtgmenu
{

class QtGMenuImporterPrivate;

class QtGMenuImporter final : public QObject
{
Q_OBJECT

public:
  QtGMenuImporter( const QString& service, const QString& path, QObject* parent = 0 );
  ~QtGMenuImporter();

  GMenu* GetGMenu() const;
  std::shared_ptr< QMenu > GetQMenu() const;

  void ForceRefresh();
  int GetItemCount();

Q_SIGNALS:
  void MenuItemsChanged();
  void MenuAppeared();
  void MenuDisappeared();

private:
  Q_DISABLE_COPY(QtGMenuImporter)
  std::unique_ptr< QtGMenuImporterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUIMPORTER_H
