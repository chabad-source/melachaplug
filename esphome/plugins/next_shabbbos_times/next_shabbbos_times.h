#pragma once


#include "../../../../plugins/hebrewcalender_melachaplug/hebrewcalender_melachaplug.h"

#include "esphome.h"
using namespace esphome;
using namespace time;


namespace NextShabbosTimes  {
    void getSunsetFromDate(time::ESPTime &esp_time, float elevation) {
        ESP_LOGD("getSunsetFromDate", esp_time.strftime("Getting sunset of: %l:%M:%S %p, %A %D").c_str());
        ESP_LOGD("getSunsetFromDate", "timestamp: %ld", esp_time.timestamp);
        esp_time.timestamp -= esp_time.hour * 60 * 60;  // change the timestamp to 12am
        ESP_LOGD("getSunsetFromDate", "Updating timestamp to be 12am: %ld", esp_time.timestamp);
        id(mysun).set_time(esp_time);                   // temporarily change date of the sun
        esp_time  = *id(mysun).sunset(elevation);       // set esp_time to sunset time

        ESP_LOGD("getSunsetFromDate", esp_time.strftime("RESULT: %l:%M:%S %p, %A %D").c_str());
        ESP_LOGD("getSunsetFromDate", "timestamp: %ld", esp_time.timestamp);
        id(mysun).set_time( id(sntp_time).now() );      // return sun to sntp time

        ESP_LOGD("getSunsetFromDate", "timestamp from the sun (should be back to cuurent date): %ld", id(mysun).sunset(elevation)->timestamp);
    }

    void getNextShabbosOnTime(time::ESPTime &esp_time, hdate &temp_hdate) {
        uint8_t counter = 0;
        while (isassurbemelachah(temp_hdate) == false) {
            // permitted to do melacha adding a day
            hdateaddday(&temp_hdate, 1);
            counter++;
        }
        while (counter > 1) {
            esp_time.increment_day();
            counter--;
        }
        // publish shabbos on time
        NextShabbosTimes::getSunsetFromDate(esp_time, id(deg_shabbos_start_global));
        id(ts_next_shabbos_on).publish_state(esp_time.strftime("%l:%M:%S %p, %A %D"));
        esp_time.increment_day();   // add a day to esptime to make it the same as hebrew date
    }
    void getNextShabbosOffTime(time::ESPTime &esp_time, hdate &temp_hdate) {
        uint8_t counter = 0;
        while (isassurbemelachah(temp_hdate) == true) {
            hdateaddday(&temp_hdate, 1);
            counter++;
        }
        while (counter > 1) {
            esp_time.increment_day();
            counter--;
        } 
        // publish shabbos off time
        NextShabbosTimes::getSunsetFromDate(esp_time, id(deg_shabbos_end_global));
        id(ts_next_shabbos_off).publish_state(esp_time.strftime("%l:%M:%S %p, %A %D"));

        esp_time.increment_day();   // add a day to esptime to make it the same as hebrew date
    }
    void getNextShabbos() {
        bool published_off_time = false;
        time::ESPTime esp_time = id(sntp_time).now();          // create temp esptime
        hdate temp_hdate = convertDate(esp_time.to_c_tm());    // create hebrew date from esptime
        if (isassurbemelachah(temp_hdate) == true) {
            // if currently assur then move the temp day to the next day that's muttar 
            NextShabbosTimes::getNextShabbosOffTime(esp_time, temp_hdate);
            published_off_time = true;  // mark shabbos off time as published
        }
        NextShabbosTimes::getNextShabbosOnTime(esp_time, temp_hdate);
        if (published_off_time == false) {
            NextShabbosTimes::getNextShabbosOffTime(esp_time, temp_hdate);
        }
    }

} // namespace NextShabbosTimes