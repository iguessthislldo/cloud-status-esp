#include <ESP8266WiFi.h>

#include "utils.h"
#include "config.h"
#include "src/esp8266-ping/src/Pinger.h"


#include <ESP8266HTTPClient.h>
extern "C" {
#include <lwip/icmp.h>
}

Output builtin_led{LED_BUILTIN, false};
WiFiClient wifi;
Pinger pinger;
bool ping_ongoing = false;
Status* current_status = nullptr;
unsigned long unit_count = 0;
const unsigned long cycle_unit = 1000;

void setup_wifi() {
    delay(10);
    Serial.printf("Wifi: Connecting to %s\n", wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        builtin_led.flash(500);
        Serial.print(".");
    }
    Serial.print("\nWiFi connected - IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(9600);
    Serial.println();

    builtin_led.init();
    setup_wifi();
    init_config();
    statuses.init_all();

    builtin_led.flash(100, 4);

    pinger.OnReceive([](const PingerResponse& response) {
        if (response.ReceivedResponse) {
            Serial.printf(
                "Reply from %s: bytes=%d time=%lums TTL=%d\n",
                response.DestIPAddress.toString().c_str(),
                response.EchoMessageSize - sizeof(struct icmp_echo_hdr),
                response.ResponseTime,
                response.TimeToLive);
        } else {
            Serial.printf("Request timed out.\n");
        }
        return true;
    });

    pinger.OnEnd([](const PingerResponse& response) {
        current_status->success = response.TotalReceivedResponses != 0;
        if (current_status->success) {
            Serial.printf("%s okay\n", current_status->name);
        } else {
            Serial.printf("%s failed\n", current_status->name);
        }
        ping_ongoing = false;
        current_status = current_status->next;
        return true;
    });
}

void loop() {
    const unsigned long start_of_loop = millis();
    if (unit_count == 0) {
        if (!ping_ongoing) {
            if (!current_status) {
                current_status = statuses.list.head;
            }
            Serial.printf("Pinging %s...\n", current_status->name);
            if (!current_status->success) {
                current_status->output.flash(100, 4);
            }
            ping_ongoing = true;
            const uint32_t timeout = 2000;
            bool success = false;
            switch (current_status->kind) {
            case Status::Gateway:
                success = pinger.Ping(WiFi.gatewayIP(), current_status->reqs, timeout);
                break;
            case Status::Address:
                success = pinger.Ping(current_status->address, current_status->reqs, timeout);
                break;
            case Status::Hostname:
                Serial.printf("%s\n", current_status->hostname);
                success = pinger.Ping(current_status->hostname, current_status->reqs, timeout);
                break;
            default:
                current_status->success = false;
                Serial.printf("No dest set\n");
            }
            if (!success) {
                Serial.printf("Failed?\n");
                current_status = current_status->next;
            }
        }
    }
    for (Status* s = statuses.list.head; s; s = s->next) {
        if (s->success) {
            s->output.set(true);
        } else {
            s->output.toggle();
        }
    }
    unit_count++;
    if (unit_count >= cycle_len) {
        unit_count = 0;
    }
    const unsigned long delta = millis() - start_of_loop;
    if (delta < cycle_unit) {
        delay(cycle_unit - delta);
    }
}
