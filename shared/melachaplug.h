#pragma once


#include "../../../../shared/hebrewcalender-melachaplug.h"

#include "esphome.h"
using namespace esphome;
using namespace time;


namespace MelachaPlug {

    hdate hdate;  // global varible for todays hebrew date

    void checkForIssurMelacha() {
        if (isassurbemelachah(hdate) == false) {
            ESP_LOGD("checkForIssurMelacha", "melacha permitted");
            // since its a weekday turn off Shabbos mode
            if (id(shabbos_mode).state == true) {
                id(shabbos_mode).turn_off(); // turn off Shabbos mode
            }
        } else {
            ESP_LOGD("checkForIssurMelacha", "melacha perhibited");
            // since its Shabbos or Yom Tov turn on Shabbos mode
            if (id(shabbos_mode).state == false) {
                id(shabbos_mode).turn_on(); // turn on Shabbos mode
            }
        }
    }

    void hdateAddDay() {
        hdateaddday(&hdate, 1); // add one day to the hdate
        id(hebrew_date).publish_state( to_string(hdate.day) + "-" + to_string(hdate.month) + "-" + to_string(hdate.year) + " d-m-y (month starts from Nissan)" );
        MelachaPlug::checkForIssurMelacha();
    }

    void hdateDeductDay() {
        hdateaddday(&hdate, -1); // deducts one day to the hdate
        id(hebrew_date).publish_state( to_string(hdate.day) + "-" + to_string(hdate.month) + "-" + to_string(hdate.year) + " d-m-y (month starts from Nissan)" );
        MelachaPlug::checkForIssurMelacha();
    }

    void convertToHdate() {
        // gets triggered on every time sync
        struct tm tm_time_now = id(sntp_time).now().to_c_tm();
        hdate = convertDate(tm_time_now);   // convert current english date to the hebrew date
        ESP_LOGD("HDATE", "Year: %d Month: %d Day: %d", hdate.year, hdate.month, hdate.day);
        id(hebrew_date).publish_state( to_string(hdate.day) + "-" + to_string(hdate.month) + "-" + to_string(hdate.year) + " d-m-y (month starts from Nissan)" );

        // since we are recalculating the hebrew day, we need to reverify and move forward the date if after Shabbos start/end time
        float deg_to_calculate;
        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == false) {
            // since its a weeekday, we calculate hebrew day by Shabbos start time
            deg_to_calculate = id(deg_shabbos_start_global);
        } else {
            // since its a Shabbos or Yom Tov, we calculate hebrew day by Shabbos end time
            deg_to_calculate = id(deg_shabbos_end_global);
        }

        auto current_time = id(sntp_time).now();
        // check if current time is after 12 noon and before midnight, and the sun elevation is lower (which happens after) then either Shabbos start or end elevation degree
        if (current_time.hour > 12 && id(sun_elevation).state < deg_to_calculate ) {
            ESP_LOGD("convertToHdate", "it's between midnight and sunset");
            MelachaPlug::hdateAddDay();  // advance hebrew date
        } else {
            ESP_LOGD("convertToHdate", "it's not between midnight and sunset");
        }

        // now that we have the proper hebrew date, we triple check to make sure Shabbos mode is on if need be
        MelachaPlug::checkForIssurMelacha();
    }  

    void onShabbosStart() {
        // gets triggered every day at the potential Shabbos or Yom Tov start time

        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == false) {
            // since its a weeekday calculate hebrew day by Shabbos start time
            MelachaPlug::hdateAddDay();  // advance hebrew date
        } else {
            ESP_LOGD("onShabbosStart", "it's Shabbos or Yom Tov, wating for later zman to advance hebrew date");
        }

        MelachaPlug::checkForIssurMelacha();
    }


    void onShabbosEnd() {
        // gets triggered every day at the potential Shabbos or Yom Tov end time

        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == true) {
            // since its a Shabbos or Yom Tov calculate hebrew day by Shabbos end time
            MelachaPlug::hdateAddDay();  // advance hebrew date
        } else {
            ESP_LOGD("onShabbosStart", "it's a weekday, date was already advanced");
        }

        MelachaPlug::checkForIssurMelacha();
    }
  
  
} // namespace MelachaPlug


