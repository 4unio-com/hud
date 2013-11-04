#ifndef QTGMENUIMPORTER_H
#define QTGMENUIMPORTER_H

#include <QObject>

class QAction;
class QDBusPendingCallWatcher;
class QIcon;
class QMenu;

namespace qtgmenu
{

class QtGMenuImporterPrivate;

class QtGMenuImporter final : public QObject
{
    Q_OBJECT

public:
    QtGMenuImporter(const QString& service, const QString& path, QObject* parent = 0);
    ~QtGMenuImporter();

    QMenu* menu() const;

public Q_SLOTS:
    void updateMenu();

Q_SIGNALS:
    void menuUpdated();
    void menuReadyToBeShown();
    void actionActivationRequested(QAction*);

protected:
    virtual QMenu* createMenu(QWidget* parent);
    virtual QIcon iconForName(const QString& name);

private Q_SLOTS:
    void sendClickedEvent(int);
    void slotMenuAboutToShow();
    void slotMenuAboutToHide();
    void slotAboutToShowDBusCallFinished(QDBusPendingCallWatcher*);
    void slotItemActivationRequested(int id, uint timestamp);
    void slotItemsPropertiesUpdated();
    void processPendingLayoutUpdates();
    void slotLayoutUpdated(uint revision, int parentId);
    void slotGetLayoutFinished(QDBusPendingCallWatcher*);

private:
    Q_DISABLE_COPY(QtGMenuImporter)
    QtGMenuImporterPrivate* const d;

    friend class QtGMenuImporterPrivate;
};

} // namespace qtgmenu

#endif /* QTGMENUIMPORTER_H */
