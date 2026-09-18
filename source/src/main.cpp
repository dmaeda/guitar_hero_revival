#include <pb_decode.h>
#include <pb_encode.h>
#include <stdint.h>
#include <stdio.h>
#include <map>
#include "main.hpp"
#include "config/config.hpp"
#include "managers/config_manager.hpp"
#include "managers/profile_manager.hpp"
#include "managers/device_manager.hpp"

#include "config.pb.h"
#include "pico/stdlib.h"
#include "config/FlashPROM.h"
#include "CRC32.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "config/config.hpp"
#include "devices/debug.hpp"
#include "common/tusb_types.h"
#include "device/usbd_pvt.h"
#include "hardware/structs/usb.h"
#include "emulation/usb/gh_arcade_device.h"
#include "emulation/usb/xinput_device.h"
#include "emulation/usb/xone_device.h"
#include "emulation/usb/ogxbox_device.h"
#include "emulation/usb/hid_device.h"
#include "emulation/usb/ps3_device.h"
#include "emulation/usb/ps4_device.h"
#include "uart.hpp"
#include "protocols/hid.hpp"
#include "main.hpp"
#include "utils.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "pico/bootrom.h"
#include "tusb.h"
#include "device/dcd.h"
#include "device/usbd_pvt.h"
#include "host/usbh.h"
#include "host/usbh_pvt.h"
#include "common/tusb_types.h"
#include "pio_usb.h"
#include "pico/stdio/driver.h"
#include "pico/stdio_uart.h"
#include <pico/cyw43_arch.h>

#include "emulation/usb/usb_descriptors.h"
#include <pico_fota_bootloader/core.h>
#include "ring_buffer.h"
#include "hci.h"
#include "devices/bt/bluetooth_stack.hpp"

namespace
{
    constexpr uint32_t UART_CONSOLE_BAUD_RATE = 115200;

    struct UartConsoleState
    {
        uint32_t clock_hz;
        uint32_t cr;
        uint32_t lcr_h;
        uint32_t ibrd;
        uint32_t fbrd;
    };

    UartConsoleState expected_uart_console;
    uint32_t uart_console_repairs = 0;

    void init_uart_console()
    {
        // This diagnostic build dedicates UART0 and GP0/GP1 to the console.
        stdio_uart_init_full(uart0, UART_CONSOLE_BAUD_RATE, 0, 1);
        uart_set_hw_flow(uart0, false, false);
        const auto *hw = uart_get_hw(uart0);
        expected_uart_console = {clock_get_hz(clk_peri), hw->cr, hw->lcr_h, hw->ibrd, hw->fbrd};
    }

    bool check_uart_console()
    {
        const auto *hw = uart_get_hw(uart0);
        const uint tx_function = gpio_get_function(0);
        const uint rx_function = gpio_get_function(1);
        const UartConsoleState current = {clock_get_hz(clk_peri), hw->cr, hw->lcr_h, hw->ibrd, hw->fbrd};
        if (tx_function == GPIO_FUNC_UART && rx_function == GPIO_FUNC_UART &&
            current.clock_hz == expected_uart_console.clock_hz &&
            current.cr == expected_uart_console.cr && current.lcr_h == expected_uart_console.lcr_h &&
            current.ibrd == expected_uart_console.ibrd && current.fbrd == expected_uart_console.fbrd)
        {
            return false;
        }
        // Let any byte still in the TX FIFO/shift register finish before the reset in
        // init_uart_console() clobbers it, otherwise in-flight output gets truncated.
        uart_tx_wait_blocking(uart0);
        init_uart_console();
        ++uart_console_repairs;
        printf("[UART0] restored altered settings: pin functions=%u/%u CR=%lx LCR=%lx divider=%lu/%lu clock=%lu\n",
               tx_function, rx_function, (unsigned long)current.cr, (unsigned long)current.lcr_h,
               (unsigned long)current.ibrd, (unsigned long)current.fbrd, (unsigned long)current.clock_hz);
        return true;
    }

    void restore_uart_console_after_load()
    {
        if (!check_uart_console())
        {
            // Preserve pending output before rebinding stdio from a possible UART1 DebugDevice.
            uart_tx_wait_blocking(uart0);
            init_uart_console();
        }
    }
}

class HidConsoleBridge
{
public:
    static constexpr size_t max_packets_per_flush = 4;

    static HidConsoleBridge& instance()
    {
        static HidConsoleBridge bridge;
        return bridge;
    }

    void init()
    {
        ring_buffer_init(&m_buffer, m_buffer_storage, sizeof(m_buffer_storage), 0);
        m_initialized = true;
    }

    void enable()
    {
        stdio_set_driver_enabled(&m_driver, true);
    }

    void disable()
    {
        stdio_set_driver_enabled(&m_driver, false);
    }

    void write(const char *buf, int len)
    {
        if (!m_initialized)
        {
            return;
        }
        ring_buffer_push(&m_buffer, buf, len);
    }

    void flush()
    {
        auto& config_mgr = ConfigManager::instance();
        if (!m_initialized || !can_send() || config_mgr.is_reloading() || config_mgr.is_working())
        {
            return;
        }
        size_t packets_sent = 0;
        while (packets_sent < max_packets_per_flush && !ring_buffer_is_empty(&m_buffer) && can_send())
        {
            tu_memclr(m_event.event.console.data, sizeof(m_event.event.console.data));
            ring_buffer_pop(&m_buffer, m_event.event.console.data, sizeof(m_event.event.console.data) - 1);
            HIDConfigDevice::send_event(m_event, true);
            packets_sent++;
        }
    }

private:
    HidConsoleBridge() = default;

    static void out_chars(const char *buf, int len)
    {
        instance().write(buf, len);
    }

    static void out_flush()
    {
        instance().flush();
    }

    static int in_chars(char *buf, int len)
    {
        return 0;
    }

    static void set_chars_available_callback(void (*fn)(void *), void *param)
    {
    }

    static bool can_send()
    {
        return !HIDConfigDevice::tool_closed();
    }

    proto_Event m_event = {which_event : proto_Event_console_tag, event : {console : {data : {}}}};
    ring_buffer_t m_buffer;
    char m_buffer_storage[1024];
    bool m_initialized = false;
    stdio_driver_t m_driver = {
        .out_chars = out_chars,
        .out_flush = out_flush,
        .in_chars = in_chars,
        .set_chars_available_callback = set_chars_available_callback,
        .next = nullptr,
        .last_ended_with_cr = true,
        .crlf_enabled = true};
};

bool mode_recently_changed()
{
    return ConfigManager::instance().mode_recently_changed(millis());
}
void hid_task(void)
{
    auto& config_mgr = ConfigManager::instance();
    
    if (config_mgr.is_working())
    {
        return;
    }
    
    ConsoleMode requested_mode = config_mgr.get_requested_mode();
    ConsoleMode current_mode = config_mgr.get_current_mode();
    
    uint32_t now = millis();
    if (config_mgr.should_reinit(now))
    {
        if (!HIDConfigDevice::tool_closed())
        {
            proto_Event event = {which_event : proto_Event_reload_tag, event : {reload : {}}};
            HIDConfigDevice::send_event(event, true);
            tud_task();
        }
        printf("requested: %d current: %d init: %d\r\n", requested_mode, current_mode, config_mgr.get_reinit_time());
        config_mgr.begin_reinit();
        load();
        restore_uart_console_after_load();
        config_mgr.finish_reinit(millis());
        return;
    }
    update();
}

void send_debug(uint8_t *data, size_t len)
{
    proto_Event event = {which_event : proto_Event_debug_tag, event : {debug : {data : {size : (pb_size_t)len}}}};
    memcpy(event.event.debug.data.bytes, data, len);
    HIDConfigDevice::send_event(event, false);
}

static void initialize_device_stack()
{
    HIDConfigDevice::reset_keepalive();
    UsbDetectionState::instance().reset();
    const tusb_rhport_init_t rh_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUD_OPT_HIGH_SPEED ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL};
    tud_rhport_init(TUD_OPT_RHPORT, &rh_init);
    if (ConfigManager::instance().has_bluetooth())
    {
        BluetoothStack::instance().power_on();
        printf("bt init done\r\n");
    }
}

void reinitialize_device_stack()
{
    printf("Reinitializing device stack\r\n");
    if (ConfigManager::instance().has_bluetooth())
    {
        BluetoothStack::instance().power_off();
        printf("bt init done\r\n");
    }
    tud_deinit(TUD_OPT_RHPORT);
    initialize_device_stack();
}

bool send_timeout = false;

void update()
{
    DeviceManager::instance().update(false, false);
    ProfileManager::instance().update(false, false);
}

void initDebug()
{
    HidConsoleBridge::instance().enable();
}

void deinitDebug()
{
    HidConsoleBridge::instance().disable();
}


void core1()
{
    multicore_lockout_victim_init();
    while (1)
    {
    }
}

namespace
{
    constexpr uint MCP3008_MISO_PIN = 16;
    constexpr uint MCP3008_CS_PIN = 17;
    constexpr uint MCP3008_SCK_PIN = 18;
    constexpr uint MCP3008_MOSI_PIN = 19;

    void init_mcp3008_diagnostics()
    {
        gpio_init(MCP3008_CS_PIN);
        gpio_put(MCP3008_CS_PIN, 1);
        gpio_set_dir(MCP3008_CS_PIN, GPIO_OUT);
        spi_init(spi0, 1000000);
        spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        gpio_set_function(MCP3008_MISO_PIN, GPIO_FUNC_SPI);
        gpio_set_function(MCP3008_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(MCP3008_MOSI_PIN, GPIO_FUNC_SPI);
        printf("MCP3008: SPI0 at 1 MHz, MISO=GP16 CS=GP17 SCK=GP18 MOSI=GP19\n");
    }

    uint16_t read_mcp3008_channel(uint8_t channel)
    {
        // Start bit, single-ended channel selection, then clocks for the result.
        const uint8_t tx[] = {0x01, static_cast<uint8_t>((0x08 | channel) << 4), 0x00};
        uint8_t rx[3] = {};
        gpio_put(MCP3008_CS_PIN, 0);
        busy_wait_us_32(1);
        spi_write_read_blocking(spi0, tx, rx, sizeof(tx));
        gpio_put(MCP3008_CS_PIN, 1);
        busy_wait_us_32(1);
        return static_cast<uint16_t>(((rx[1] & 0x03) << 8) | rx[2]);
    }
}

int main()
 {
    // Configure the peripheral clock before calculating the UART baud divider.
    set_sys_clock_khz(180000, true);
    init_uart_console();
    uart_puts(uart0, "UART0 ready: GP0 TX, GP1 RX, 115200 8N1\r\n");
    //printf("UART diagnostic build: %s %s\n", __DATE__, __TIME__);
    printf("main: boot start\r\n");
    if (pfb_is_after_firmware_update())
    {
        // handle new firmare info if needed
    }
    if (pfb_is_after_rollback())
    {
        // handle performed rollback if needed
    }
    pfb_firmware_commit();
    ConfigManager::instance().sync_requested_mode_to_current();
    multicore_launch_core1(core1);
    adc_init();
    HidConsoleBridge::instance().init();
    EEPROM.start();
    if (!load())
    {
        printf("main: load() failed, falling back to load_empty()\r\n");
        // config was not valid, save a empty config
        load_empty();
        // load();
    }
    else
    {
        printf("main: load() succeeded\r\n");
    }
    //printf("init %d\r\n", ConfigManager::instance().get_current_mode());
    initialize_device_stack();
    ConfigManager::instance().finish_reinit(millis());
    init_mcp3008_diagnostics();
    restore_uart_console_after_load();
    //printf("[UART0] console ready after device setup; build %s %s\n", __DATE__, __TIME__);
    
    uint32_t last_uart_heartbeat = millis();
    uint32_t last_mcp3008_print = millis();
    while (1)
    {
        tud_task(); // tinyusb device task
        tuh_task();
        hid_task();
        HidConsoleBridge::instance().flush();
        EEPROM.tick();
        // Temporary diagnostics: compare direct UART output with stdio output.
        const uint32_t now = millis();
        if (now - last_mcp3008_print >= 500)
        {
            last_mcp3008_print = now;
            check_uart_console();
            uint16_t channels[8];
            for (uint8_t channel = 0; channel < 8; ++channel)
            {
                channels[channel] = read_mcp3008_channel(channel);
            }
            printf("[MCP3008] CH0=%d CH1=%d CH2=%d CH3=%d CH4=%d CH5=%d CH6=%d CH7=%d\n",
                   channels[0], channels[1], channels[2], channels[3],
                   channels[4], channels[5], channels[6], channels[7]);
        }
        if (now - last_uart_heartbeat >= 2000)
        {
            last_uart_heartbeat = now;
            uart_puts(uart0, "[UART0] alive\r\n");
            printf("[stdio] alive; GP0 UART=%d, GP1 UART=%d, repairs=%lu\n",
                   gpio_get_function(0) == GPIO_FUNC_UART,
                   gpio_get_function(1) == GPIO_FUNC_UART,
                   (unsigned long)uart_console_repairs);
        }
    }
    return 0;
}
