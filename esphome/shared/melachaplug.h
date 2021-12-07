#pragma once


#include "../../../../shared/hebrewcalender-melachaplug.h"

#include "esphome.h"
using namespace esphome;
using namespace time;


namespace MelachaPlug {

    hdate hdate;  // global varible for todays hebrew date

    void checkForIssurMelacha() {
        // should only be run once were are certain the hebrew date is correct

        if (isassurbemelachah(hdate) == false) {
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
        hdateaddday(&hdate, 1); // add one day to the hdate
        id(hebrew_date).publish_state( to_string(hdate.day) + "-" + to_string(hdate.month) + "-" + to_string(hdate.year) + " d-m-y (month starts from Nissan)" );
        MelachaPlug::checkForIssurMelacha();
    }
    
    // never really used
    void hdateDeductDay() {
        hdateaddday(&hdate, -1); // deducts one day to the hdate
        id(hebrew_date).publish_state( to_string(hdate.day) + "-" + to_string(hdate.month) + "-" + to_string(hdate.year) + " d-m-y (month starts from Nissan)" );
        MelachaPlug::checkForIssurMelacha();
    }

    void convertToHdate() {
        // gets triggered on every time sync
        auto current_time = id(sntp_time).now();
        struct tm tm_time_now = current_time.to_c_tm();
        hdate = convertDate(tm_time_now);   // convert current english date to the hebrew date
        ESP_LOGD("HDATE", "Year: %d Month: %d Day: %d", hdate.year, hdate.month, hdate.day);
        id(hebrew_date).publish_state( to_string(hdate.day) + "-" + to_string(hdate.month) + "-" + to_string(hdate.year) + " d-m-y (month starts from Nissan)" );

        // since we are recalculating the hebrew day, we need to reverify and move forward the date if it's after Shabbos start/end time
        float deg_to_calculate;
        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == false) {
            // since its a weeekday, we calculate hebrew day starting from Shabbos start time
            deg_to_calculate = id(deg_shabbos_start_global);
        } else {
            // since its a Shabbos or Yom Tov, we calculate hebrew day starting from Shabbos end time
            deg_to_calculate = id(deg_shabbos_end_global);
        }
        
        // check if current time is after 12 noon (and before midnight), and the sun elevation is lower (which happens after) then Shabbos start/end elevation degree
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

    void autoSetLocation() {
        HTTPClient http;
        http.begin("http://ip-api.com/json/?fields=status,regionName,city,zip,lat,lon"); // API from http://ip-api.com/
        int httpResponseCode = http.GET();
        std::string payload = http.getString().c_str();
        ESP_LOGD("autoSetLocation", "Curernt Location: %s", payload.c_str());
        http.end();
        
        int start = payload.find("status") + 9;
        int end = payload.find(",", start) - 1;
        std::string status = payload.substr(start, end - start);
        
        if(status == "success") {
            start = payload.find("regionName") + 13;
            end = payload.find(",", start) - 1;
            std::string state = payload.substr(start, end - start);
            
            start = payload.find("city") + 7;
            end = payload.find(",", start) - 1;
            std::string city = payload.substr(start, end - start);
            
            start = payload.find("zip") + 6;
            end = payload.find(",", start) - 1;
            std::string zip = payload.substr(start, end - start);

            start = payload.find("lat") + 5;
            end = payload.find(",", start);
            std::string lat = payload.substr(start, end - start);
            // float lat = parse_float(payload.substr(start, end - start)).value();
            
            start = payload.find("lon") + 5;
            end = payload.find("}", start); // it's the end of the json therefor look for '}'
            std::string lon = payload.substr(start, end - start);
            // float lon = parse_float(payload.substr(start, end - start)).value();
            
            ESP_LOGD("autoSetLocation", "status: %s, state: %s, city: %s, zip: %s, lat: %s, lon: %s", status.c_str(), state.c_str(), city.c_str(), zip.c_str(), lat.c_str(), lon.c_str());
            id(auto_location_found).publish_state("Your in: " + city + ", " + state + " " + zip);
            id(lat_text_sensor).publish_state(lat);
            id(lon_text_sensor).publish_state(lon);
        } else {
            id(auto_location_found).publish_state("Failed to get your location");
            ESP_LOGD("autoSetLocation", "Failed to get your location");
        }
    }

    void onBoot() {
        // check if manual location is set to false
        if (id(manual_location) == false) {
            // check if the global latitude or longitude is blank (or 0)
            if (id(lat_global) || id(lon_global) != true) {
                MelachaPlug::autoSetLocation();
            } else {
                // set the location based off the saved globals
                id(lat_text_sensor).publish_state( to_string( id(lat_global) ) );
                id(lon_text_sensor).publish_state( to_string( id(lon_global) ) );
            }
        }
    }

} // namespace MelachaPlug