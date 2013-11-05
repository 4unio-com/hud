#ifndef QTGMENUIMPORTER_H
#define QTGMENUIMPORTER_H

#include <QObject>
#include <memory>

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

    std::shared_ptr<QMenu> Menu() const;

Q_SIGNALS:
    void menuUpdated();
    void menuReadyToBeShown();
    void actionActivationRequested(QAction*);

private:
    Q_DISABLE_COPY(QtGMenuImporter)
    std::unique_ptr<QtGMenuImporterPrivate> d;

    friend class QtGMenuImporterPrivate;
};

} // namespace qtgmenu

#endif /* QTGMENUIMPORTER_H */
