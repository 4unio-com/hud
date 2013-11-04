#include <QtGMenuDBus.h>
#include <QtGMenuDBusAdaptor.h>

#include <common/Localisation.h>

#undef signals
#include <gio/gio.h>

QtGMenuDBus::QtGMenuDBus(const QDBusConnection& connection, QObject* parent)
    : QObject(parent),
      m_connection(connection),
      m_adaptor(new QtGMenuDBusAdaptor(this))
{
    if (!m_connection.registerObject("/com/canonical/qtgmenu", this))
    {
        throw std::logic_error(_("Unable to register QtGMenu object on DBus"));
    }
    if (!m_connection.registerService("com.canonical.qtgmenu"))
    {
        throw std::logic_error(_("Unable to register QtGMenu service on DBus"));
    }
}

QtGMenuDBus::~QtGMenuDBus()
{
    m_connection.unregisterObject("/com/canonical/qtgmenu");
}

bool QtGMenuDBus::IsPositiveInt(int integer)
{
    m_adaptor->Calculating();
    return integer > 0;
}
