#ifdef DEVICE_DISPLAY_MODULE

#pragma once
#include "Widget.h"

class WidgetIPRouter : public Widget
{
  public:
    const std::string logPrefix() { return "Widget IP-Router"; }
    WidgetIPRouter(uint32_t displayTime, WidgetFlags action);

    void start() override;
    void stop() override;
    void pause() override;
    void resume() override;
    void setup() override;
    void loop() override;

    inline const WidgetState getState() const override { return _state; }
    inline const std::string getName() const override { return _name; }
    inline void setName(const std::string &name) override { _name = name; }

    uint32_t getDisplayTime() const override;
    WidgetFlags getAction() const override;

    inline void setDisplayTime(uint32_t displayTime) override { _displayTime = displayTime; }
    inline void setAction(uint8_t action) override { _action = static_cast<WidgetFlags>(action); }
    inline void addAction(uint8_t action) override { _action = static_cast<WidgetFlags>(_action | action); }
    inline void removeAction(uint8_t action) override { _action = static_cast<WidgetFlags>(_action & ~action); }

    void setDisplayModule(i2cDisplay *displayModule) override;
    i2cDisplay *getDisplayModule() const override;

    // Pages: page 0 is the status card; without auto paging the rotation shows only page 0.
    size_t getPageCount() const override { return PAGE_COUNT; }
    size_t getCurrentPage() const override { return _page; }
    void setPage(size_t page) override;
    void nextPage() override { setPage((_page + 1) % PAGE_COUNT); }
    void prevPage() override { setPage((_page + PAGE_COUNT - 1) % PAGE_COUNT); }

  private:
    enum Page : uint8_t
    {
        PAGE_CARD = 0,
        PAGE_IP,
        PAGE_COUPLER,
        PAGE_ROUTING,
        PAGE_COUNT
    };
    size_t _page = 0;
    bool _forceRedraw = true;

    WidgetState _state;
    uint32_t _displayTime;
    WidgetFlags _action;
    i2cDisplay *_display;

    static constexpr uint32_t REDRAW_INTERVAL_MS = 500;
    uint32_t _lastDraw = 0;
    std::string _name = "IP-Router";

    void drawCard(Adafruit_SSD1306 &d);
    void drawIPInfo(Adafruit_SSD1306 &d);
    void drawCoupler(Adafruit_SSD1306 &d);
    void drawRouting(Adafruit_SSD1306 &d);
};

#endif // DEVICE_DISPLAY_MODULE
