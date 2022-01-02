#pragma once

#include "../../../../plugins/hdate/hdate.h"

#include "esphome.h"
using namespace esphome;
using namespace time;

namespace RandomMinute  {

    void generateRandomMinute() {
        // runs on every shabbos start time (and on boot)
        time_t timestamp_start = id(sntp_time).timestamp_now();
        time_t timestamp_end = id(mysun).sunset( id(deg_shabbos_end_global) )->timestamp;

        // check if currently it's between Shabbos start and end elevation degree
        if (id(sntp_time).now().hour > 12 && id(mysun).elevation() > id(deg_shabbos_start_global) && id(mysun).elevation() < id(deg_shabbos_end_global)) {
            ESP_LOGD("RandomMinute::generateRandomMinute", "in between shabbos start-end elevation..adding a day");
            timestamp_end = timestamp_end + 86400;  // add a day
        } else {
            ESP_LOGD("RandomMinute::generateRandomMinute", "this should only run on boot (if shabbos mode is on)");
        }
        ESP_LOGD("RandomMinute::generateRandomMinute", "timestamp_start: %ld timestamp_end: %ld", timestamp_start, timestamp_end);
        long int random_number = random(timestamp_start, timestamp_end);
        ESP_LOGD("RandomMinute::generateRandomMinute", "random_number: %ld", random_number);
        random_number -= random_number % 60;  // rounds down minute to 0 seconds
        ESP_LOGD("RandomMinute::generateRandomMinute", "FIXED random_number: %ld", random_number);

        if (id(random_time_1) == 0){
            id(random_time_1) = random_number;
        } else {
            id(random_time_2) = random_number;
        }
    }

    void relayMomentarilyTurnOff() {
        ESP_LOGD("RandomMinute::relayMomentarilyTurnOff", "Turning the relay momentarily off");
        id(template_relay).turn_off();
    }


    // -------------------------------------------------------------------------------------
    void onBoot() {
        // in case of reboot on Shabbos or Yom Tov
        if (isassurbemelachah(Hdate::current_hdate) == true) {
            RandomMinute::generateRandomMinute();
        }
    }

    void onInterval() {
        if (id(hdate_valid) && id(shabbos_mode).state == true) {
            time_t timestamp_current = id(sntp_time).timestamp_now();
            timestamp_current -= timestamp_current % 60;  // rounds down minute to 0 seconds
            ESP_LOGD("RandomMinute::onInterval", "FIXED timestamp_current: %ld", timestamp_current);

            if (timestamp_current == id(random_time_1) ) {
                id(random_time_1) = 0; // reset time
                id(last_random).publish_state(to_string(timestamp_current));
                RandomMinute::relayMomentarilyTurnOff();
            } else if (timestamp_current == id(random_time_2) ) {
                id(random_time_2) = 0; // reset time
                id(last_random).publish_state(to_string(timestamp_current));
                RandomMinute::relayMomentarilyTurnOff();
            }
        }
    }

    void onShabbosStart() {
        ESP_LOGD("RandomMinute::onShabbosStart", "ran------");
        if (isassurbemelachah(Hdate::current_hdate) == true) {
            // runs whether it's the first day or not
            ESP_LOGD("RandomMinute::onShabbosStart", "Generating random minute");
            RandomMinute::generateRandomMinute();
        }
    }
    void testing() {
        ESP_LOGD("RandomMinute::test", "ran------------------------");
    }
} // namespace RandomMinute

namespace Test2  {
    void test2() {
        ESP_LOGD("Test2::test2", "ran------------------------");
    }
}