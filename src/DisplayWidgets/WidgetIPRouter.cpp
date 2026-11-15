#ifdef DEVICE_DISPLAY_MODULE
#include "WidgetIPRouter.h"
#include "DeviceDisplay.h"
#include "DisplayUi.h"
#include "IPRouterModule.h"
#include "Icons/UiIcons.h"
#include "NetworkModule.h"
#include "OpenKNX.h"
#include <WiFi.h>

namespace Ui = OpenKNX::Ui;

WidgetIPRouter::WidgetIPRouter(uint32_t displayTime, WidgetFlags action)
    : _state(WidgetState::STOPPED), _displayTime(displayTime), _action(action), _display(nullptr)
{
}

void WidgetIPRouter::setup()
{
    logDebugP("Setup...");
    if (_display == nullptr)
    {
        logErrorP("Display is NULL.");
        return;
    }
}

void WidgetIPRouter::start()
{
    if (_state == WidgetState::RUNNING)
        return;

    logDebugP("Starting...");
    _state = WidgetState::RUNNING;
    _forceRedraw = true;
}

void WidgetIPRouter::stop()
{
    logDebugP("Stopping...");
    _state = WidgetState::STOPPED;
    if (_display)
    {
        _display->display->clearDisplay();
        _display->displayBuff();
    }
}

void WidgetIPRouter::pause()
{
    if (_state == WidgetState::RUNNING)
    {
        logDebugP("Pausing...");
        _state = WidgetState::PAUSED;
    }
}

void WidgetIPRouter::resume()
{
    if (_state == WidgetState::PAUSED)
    {
        logDebugP("Resuming");
        _state = WidgetState::RUNNING;
        _forceRedraw = true;
    }
}

void WidgetIPRouter::setPage(size_t page)
{
    if (page >= PAGE_COUNT)
        page = PAGE_COUNT - 1;
    if (page != _page)
    {
        _page = page;
        _forceRedraw = true;
    }
}

void WidgetIPRouter::loop()
{
    if (_state != WidgetState::RUNNING || !_display)
        return;

    // Rebuild at most every REDRAW_INTERVAL_MS, at once after a page change.
    const uint32_t now = millis();
    if (!_forceRedraw && now - _lastDraw < REDRAW_INTERVAL_MS)
        return;
    _lastDraw = now;
    _forceRedraw = false;

    Adafruit_SSD1306 &d = *_display->display;
    Ui::begin(d);
    d.clearDisplay();
    switch (_page)
    {
        case PAGE_IP: drawIPInfo(d); break;
        case PAGE_COUPLER: drawCoupler(d); break;
        case PAGE_ROUTING: drawRouting(d); break;
        case PAGE_CARD:
        default: drawCard(d); break;
    }
    _display->displayBuff();
}

uint32_t WidgetIPRouter::getDisplayTime() const
{
    return _displayTime;
}
WidgetFlags WidgetIPRouter::getAction() const
{
    return _action;
}

void WidgetIPRouter::setDisplayModule(i2cDisplay *displayModule)
{
    _display = displayModule;
}
i2cDisplay *WidgetIPRouter::getDisplayModule() const
{
    return _display;
}

static const unsigned char PROGMEM x_icon[] = {
    0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81};

/** @brief Dotted IPv4 into buf without the heap String of IPAddress::toString(). */
static const char *formatAddr(const IPAddress &a, char *buf, size_t n)
{
    snprintf(buf, n, "%u.%u.%u.%u", (unsigned)a[0], (unsigned)a[1], (unsigned)a[2], (unsigned)a[3]);
    return buf;
}

#ifdef KNX_TUNNELING
// Format a host-order IPv4 (MSB = first octet, matching the tunnel server's own print) into buf.
static void formatIp(uint32_t ip, char *buf, size_t n)
{
    snprintf(buf, n, "%u.%u.%u.%u",
             (unsigned)((ip >> 24) & 0xFF), (unsigned)((ip >> 16) & 0xFF),
             (unsigned)((ip >> 8) & 0xFF), (unsigned)(ip & 0xFF));
}
#endif

/** @brief Card symbol: IP and TP boxes with arrows both ways, in x44..83 y1..27. */
static void drawIpTpSymbol(Adafruit_SSD1306 &d)
{
    d.drawRoundRect(44, 3, 17, 22, 2, WHITE);
    d.setCursor(47, 10);
    d.print("IP");
    d.drawRoundRect(67, 3, 17, 22, 2, WHITE);
    d.setCursor(70, 10);
    d.print("TP");
    d.drawFastHLine(61, 9, 6, WHITE);
    d.fillTriangle(66, 9, 64, 7, 64, 11, WHITE);
    d.drawFastHLine(61, 17, 6, WHITE);
    d.fillTriangle(61, 17, 63, 15, 63, 19, WHITE);
}

void WidgetIPRouter::drawCard(Adafruit_SSD1306 &d)
{
    const uint8_t *badge = Ui::kBadgeError;
    const char *word = "No bus";
    const char *detail = "No TP link";
    char b[24];

    TpUartDataLinkLayer *dll = KNX_TP_DLL; // the secondary link on the router BAU
    if (!openknxNetwork.established())
    {
        word = "No LAN";
        detail = "Check cable";
    }
    else if (dll)
    {
        switch (dll->getTPUart().getBcuState())
        {
            case TPUart::BCU_DISCONNECTED:
                detail = "Check KNX line";
                break;
            case TPUart::BCU_UNINITIALIZED:
                // Only the first 30 s after boot count as start-up; later an uninitialised BCU means no bus.
                if (uptime() >= 30) // wrap-safe seconds since boot
                {
                    detail = "Check KNX line";
                    break;
                }
                badge = Ui::kBadgeClock;
                word = "Starting";
                detail = "TP starting";
                break;
            default:
            {
                // Bus power loss: bus-powered chip after ~3 unanswered probes, external NCN via VBUS; BCU state lags.
                if (!dll->getTPUart().busOperational())
                {
                    detail = "Check KNX line";
                    break;
                }
                const std::string load = humanBytes(openknx.busLoad.currentBytesPerSec());
#ifdef KNX_TUNNELING
                IpTunnelServer &ts = knx.bau().getIpTunnelServer();
                const uint8_t n = ts.tunnelCount();
                const KnxIpTunnelConnection *t = n == 1 ? ts.tunnelAt(0) : nullptr;
                if (t)
                {
                    // One client: who it is; the load only while both fit the line.
                    char ip[16];
                    formatIp(t->IpAddress, ip, sizeof(ip));
                    if (snprintf(b, sizeof(b), "%s  %s/s", ip, load.c_str()) > 21)
                        snprintf(b, sizeof(b), "%s", ip);
                }
                else
                    snprintf(b, sizeof(b), "%u %s  %s/s", (unsigned)n, n == 1 ? "tunnel" : "tunnels", load.c_str());
#else
                snprintf(b, sizeof(b), "%s/s", load.c_str());
#endif
                // A router routes all the time; the tunnels are the detail, not the state.
                badge = Ui::kBadgeOk;
                word = "Routing";
                detail = b;
                break;
            }
        }
    }

    Ui::statusCard(d, drawIpTpSymbol, badge, word, detail);
    Ui::pageDots(d, PAGE_COUNT, PAGE_CARD);
}

void WidgetIPRouter::drawIPInfo(Adafruit_SSD1306 &d)
{
    Ui::header(d, _name.c_str(), PAGE_COUNT, PAGE_IP);

    if (openknxNetwork.established())
    {
        char b[16]; // "255.255.255.255"
        Ui::row(d, 0, "IP", formatAddr(openknxNetwork.localIP(), b, sizeof(b)));
        Ui::row(d, 1, "GW", formatAddr(openknxNetwork.gatewayIP(), b, sizeof(b)));
        Ui::row(d, 2, "DNS", formatAddr(openknxNetwork.nameServerIP(), b, sizeof(b)));
    }
    else
    {
        d.drawBitmap(d.width() / 2 - 4, 20, x_icon, 8, 8, WHITE);
        Ui::printCentred(d, 34, "Disconnected");
        Ui::printCentred(d, 44, "Check LAN Cable");
    }
}

// Role from the own address, routing group, TTL, tunnel use.
void WidgetIPRouter::drawCoupler(Adafruit_SSD1306 &d)
{
    Ui::header(d, "Coupler", PAGE_COUNT, PAGE_COUPLER);

    char b[24];
    const uint16_t pa = knx.individualAddress();
    const unsigned area = pa >> 12, line = (pa >> 8) & 0x0F, dev = pa & 0xFF;
    if (!knx.configured())
        snprintf(b, sizeof(b), "Not programmed"); // the coupler default 15.15.0 would read as a line coupler
    else if (dev != 0)
        snprintf(b, sizeof(b), "No coupler %u.%u.%u", area, line, dev); // a device part: misconfigured
    else if (line == 0)
        snprintf(b, sizeof(b), "Area coupler %u.0", area);
    else
        snprintf(b, sizeof(b), "Line coupler %u.%u", area, line);
    Ui::row(d, 0, b, nullptr);

    const char *group = IPRouterModule::routingGroup();
    Ui::row(d, 1, "Group", group[0] ? group : "-", 8);

    const int ttl = IPRouterModule::routingTtl();
    if (ttl < 0)
        snprintf(b, sizeof(b), "-");
    else
        snprintf(b, sizeof(b), "%d", ttl);
    Ui::row(d, 2, "TTL", b, 8);

#ifdef KNX_TUNNELING
    IpTunnelServer &ts = knx.bau().getIpTunnelServer();
    snprintf(b, sizeof(b), "%u / %u", (unsigned)ts.tunnelCount(), (unsigned)ts.tunnelMax());
    Ui::row(d, 3, "Tunnels", b, 8);
#endif
}

// Telegrams per direction: routed, dropped by the filter table, not delivered on that side (tunnel losses included).
void WidgetIPRouter::drawRouting(Adafruit_SSD1306 &d)
{
    Ui::header(d, "Routing", PAGE_COUNT, PAGE_ROUTING);

    KnxIpCounters &c = knx.bau().getCounters();
    const uint32_t v[6] = {c.routedToKnx(), c.routedToIp(), c.filteredToKnx(), c.filteredToIp(), c.lostToKnx(), c.lostToIp()};
    static const char *const label[3] = {"Routed", "Filter", "Lost"};
    // Columns under "IP>TP" and "TP>IP": humanCount() of a 32-bit counter is at most 5 characters.
    Ui::row(d, 0, nullptr, "IP>TP  TP>IP", 8);
    char b[16];
    for (uint8_t i = 0; i < 3; i++)
    {
        snprintf(b, sizeof(b), "%5s  %5s", humanCount(v[2 * i]).c_str(), humanCount(v[2 * i + 1]).c_str());
        Ui::row(d, (uint8_t)(i + 1), label[i], b, 8);
    }
}

#endif // DEVICE_DISPLAY_MODULE
