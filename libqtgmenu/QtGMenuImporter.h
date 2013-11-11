#ifndef QTGMENUIMPORTER_H
#define QTGMENUIMPORTER_H

#include <QObject>
#include <memory>

class QAction;
class QDBusPendingCallWatcher;
class QIcon;
class QMenu;

class _GMenuModel;
typedef _GMenuModel GMenuModel;

class _GActionGroup;
typedef _GActionGroup GActionGroup;

namespace qtgmenu
{

class QtGMenuImporterPrivate;

class QtGMenuImporter final : public QObject
{
Q_OBJECT

public:
  QtGMenuImporter( const QString& service, const QString& path, QObject* parent = 0 );
  ~QtGMenuImporter();

  GMenuModel* GetGMenuModel() const;
  GActionGroup* GetGActionGroup() const;

  std::shared_ptr< QMenu > GetQMenu() const;

  void ForceRefresh();

Q_SIGNALS:
  void MenuItemsChanged( int position, int removed, int added );
  void MenuAppeared();
  void MenuDisappeared();

  void ActionAdded( QString action_name );
  void ActionEnabled( QString action_name, bool enabled );
  void ActionRemoved( QString action_name );
  void ActionStateChanged( QString action_name, QVariant value );
  void ActionsAppeared();
  void ActionsDisappeared();

private:
  Q_DISABLE_COPY(QtGMenuImporter)
  std::unique_ptr< QtGMenuImporterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUIMPORTER_H
