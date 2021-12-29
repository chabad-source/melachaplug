#pragma once

#include "../../../../plugins/hdate/hdate.h"

#include "esphome.h"
using namespace esphome;
using namespace time;

namespace MelachaPlug  {
    void updateTextSensors() {
        id(ts_shabbos_start).update();
        id(ts_sunset).update();
        id(ts_tzeit).update();
        id(ts_shabbos_end).update();
    }

    void publishHebrewDay() {
        std::string hebrew_month;
        switch(Hdate::current_hdate.month) {
        case 1:
            hebrew_month = "Nissan";
            break;
        case 2:
            hebrew_month = "Iyar";
            break;
        case 3:
            hebrew_month = "Sivan";
            break;
        case 4:
            hebrew_month = "Tammuz";
            break;
        case 5:
            hebrew_month = "Av";
            break;
        case 6:
            hebrew_month = "Elul";
            break;
        case 7:
            hebrew_month = "Tishrei";
            break;
        case 8:
            hebrew_month = "Cheshvan";
            break;
        case 9:
            hebrew_month = "Kislev";
            break;
        case 10:
            hebrew_month = "Tevet";
            break;
        case 11:
            hebrew_month = "Shevat";
            break;
        case 12:
            if (Hdate::current_hdate.leap) {
                hebrew_month = "Adar I";
            } else {
                hebrew_month = "Adar";
            }
            break;
        case 13:
            hebrew_month = "Adar II";
            break;
        default:
            ESP_LOGD("publishHebrewDay", "no month found in hebrew date");
        }

        id(hebrew_date).publish_state( to_string(Hdate::current_hdate.day) + " " + hebrew_month + " " + to_string(Hdate::current_hdate.year) );
        ESP_LOGD("publishHebrewDay", "Year: %d Month: %s Day: %d", Hdate::current_hdate.year, hebrew_month.c_str(), Hdate::current_hdate.day);
    }

    void checkForIssurMelacha() {
        // should only be run once were are certain the hebrew date is correct (and we know the current sun elevation)
        if (isassurbemelachah(Hdate::current_hdate) == false) {
            ESP_LOGD("checkForIssurMelacha", "melacha permitted");
            // since its a weekday turn off Shabbos mode
            if (id(shabbos_mode).state == true) {
                id(shabbos_mode).turn_off(); // turn off Shabbos mode
            }
        } else {
            ESP_LOGD("checkForIssurMelacha", "melacha prohibited");
            // since its Shabbos or Yom Tov turn on Shabbos mode
            if (id(shabbos_mode).state == false) {
                id(shabbos_mode).turn_on(); // turn on Shabbos mode
            }
        }
    }

    void hdateAddDay() {
        hdateaddday(&Hdate::current_hdate, 1);                       // advance hdate
    }
    
    // // never really used
    // void hdateDeductDay() {
    //     hdateaddday(&Hdate::current_hdate, -1);                      // deducts one day to the hdate
    // }

    void convertToHdate() {
        // only run once we have the proper time and sun state
        auto current_time = id(sntp_time).now();              // we use  this variable later
        struct tm tm_time_now = current_time.to_c_tm();
        Hdate::current_hdate = convertDate(tm_time_now);             // convert current english date to the hebrew date

        // since we are recalculating the hebrew day, we need to reverify and
        // move forward the date if it's after Shabbos start/end time
        float deg_to_calculate;
        if (isassurbemelachah(Hdate::current_hdate) == false) {
            // since its a weeekday, we calculate hebrew day
            // starting from Shabbos start time
            deg_to_calculate = id(deg_shabbos_start_global);
            ESP_LOGD("convertToHdate", "since its a weeekday, we calculate hebrew day starting from Shabbos start time");
        } else {
            // since its Shabbos or Yom Tov, we calculate
            // hebrew day starting from Shabbos end time
            deg_to_calculate = id(deg_shabbos_end_global);
            ESP_LOGD("convertToHdate", "since its Shabbos or Yom Tov, we calculate hebrew day starting from Shabbos end time");
        }
        ESP_LOGD("convertToHdate", "current sun elevation: %f degree to calculate: %f", id(mysun).elevation(), deg_to_calculate);

        // check if current time is after 12 noon (and before midnight), and the sun elevation
        // is lower (which happens after) then Shabbos start/end elevation degree
        if (current_time.hour > 12 && id(mysun).elevation() < deg_to_calculate ) {
            MelachaPlug::hdateAddDay();
            ESP_LOGD("convertToHdate", "it's between midnight and sunset");
        } else {
            ESP_LOGD("convertToHdate", "it's not between midnight and sunset");
        }

        id(hdate_valid) = true;
    }


    // ---------------------------------------- On Sequences ----------------------------------------
    void onTimeSync() {
        ESP_LOGD("onTimeSync", "onTimeSync ran");
        // re-update the hebrew date
        MelachaPlug::convertToHdate();
        // now that we have the proper hebrew date, we triple check to make sure Shabbos mode is on if need be
        MelachaPlug::checkForIssurMelacha();
        MelachaPlug::publishHebrewDay();
        MelachaPlug::updateTextSensors();
    }

    void onShabbosStart() {
        // gets triggered every day at the potential Shabbos or Yom Tov start time
        if (isassurbemelachah(Hdate::current_hdate) == false) {
            // since its a weeekday calculate hebrew day by Shabbos start time
            MelachaPlug::hdateAddDay();
            MelachaPlug::checkForIssurMelacha();
            MelachaPlug::publishHebrewDay();
            MelachaPlug::updateTextSensors();
        } else {
            ESP_LOGD("onShabbosStart", "it's Shabbos or Yom Tov, wating for later zman to advance hebrew date");
        }
    }

    void onShabbosEnd() {
        // gets triggered every day at the potential Shabbos or Yom Tov end time
        if (isassurbemelachah(Hdate::current_hdate) == true) {
            // since its a Shabbos or Yom Tov calculate hebrew day by Shabbos end time
            MelachaPlug::hdateAddDay();
            MelachaPlug::checkForIssurMelacha();
            MelachaPlug::publishHebrewDay();
            MelachaPlug::updateTextSensors();
        } else {
            ESP_LOGD("onShabbosStart", "it's a weekday, date was already advanced");
        }
    }
} // namespace MelachaPlug