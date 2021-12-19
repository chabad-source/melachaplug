#pragma once


#include "../../../../shared/hebrewcalender-melachaplug.h"

#include "esphome.h"
using namespace esphome;
using namespace time;


namespace MelachaPlug {
    hdate hdate;  // global varible for todays hebrew date

    void updateTextSensors() {
        id(ts_shabbos_start).update();
        id(ts_sunset).update();
        id(ts_tzeit).update();
        id(ts_shabbos_end).update();
    }

    void publishHebrewDay() {
        std::string hebrew_month;
        switch(hdate.month) {
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
            if (hdate.leap) {
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

        id(hebrew_date).publish_state( to_string(hdate.day) + " " + hebrew_month + " " + to_string(hdate.year) );
        ESP_LOGD("publishHebrewDay", "Year: %d Month: %s Day: %d", hdate.year, hebrew_month.c_str(), hdate.day);
    }

    void checkForIssurMelacha() {
        // should only be run once were are certain the hebrew date is correct (and we know the current sun elevation)

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
    }
    
    // never really used
    void hdateDeductDay() {
        hdateaddday(&hdate, -1); // deducts one day to the hdate
    }

    void convertToHdate() {
        // only run if we know have the proper time and sun state

        auto current_time = id(sntp_time).now();
        struct tm tm_time_now = current_time.to_c_tm();
        hdate = convertDate(tm_time_now);   // convert current english date to the hebrew date

        // since we are recalculating the hebrew day, we need to reverify and move forward the date if it's after Shabbos start/end time
        float deg_to_calculate;
        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == false) {
            // since its a weeekday, we calculate hebrew day starting from Shabbos start time
            deg_to_calculate = id(deg_shabbos_start_global);
            ESP_LOGD("convertToHdate", "since its a weeekday, we calculate hebrew day starting from Shabbos start time");
        } else {
            // since its a Shabbos or Yom Tov, we calculate hebrew day starting from Shabbos end time
            deg_to_calculate = id(deg_shabbos_end_global);
            ESP_LOGD("convertToHdate", "since its a Shabbos or Yom Tov, we calculate hebrew day starting from Shabbos end time");
        }
        ESP_LOGD("convertToHdate", "current sun elevation: %f degree where we calculate from: %f", id(mysun).elevation(), deg_to_calculate);

        // check if current time is after 12 noon (and before midnight), and the sun elevation is lower (which happens after) then Shabbos start/end elevation degree
        if (current_time.hour > 12 && id(mysun).elevation() < deg_to_calculate ) {
            MelachaPlug::hdateAddDay();  // advance hebrew date
            ESP_LOGD("convertToHdate", "it's between midnight and sunset");
        } else {
            ESP_LOGD("convertToHdate", "it's not between midnight and sunset");
        }
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
            id(lat_text_sensor).publish_state(lat);
            id(lon_text_sensor).publish_state(lon);
            id(auto_location_found).publish_state("Auto detected: " + city + ", " + state + " " + zip);
        } else {
            id(auto_location_found).publish_state("Failed to get your location");
            ESP_LOGD("autoSetLocation", "Failed to get your location");
        }
    }


    // -------------------------------------------------------------------------------------


    void onBoot() {
        ESP_LOGD("onBoot", "--------- sun elevation: %f", id(mysun).elevation() );
        ESP_LOGD("onBoot", "lat: %f lon: %f", id(lat_global), id(lon_global) );

        if (id(lat_global) == 0.0f || id(lon_global) == 0.0f) {
            // use preprogrammed location
            int separator = id(lat_and_lon_global).find(",");
            // std::string orig_lat = id(lat_and_lon_global).substr(0, separator);
            // std::string orig_lon = id(lat_and_lon_global).substr(separator +1, id(lat_and_lon_global).length());
            id(lat_text_sensor).publish_state( id(lat_and_lon_global).substr(0, separator) );
            id(lon_text_sensor).publish_state( id(lat_and_lon_global).substr( separator +1, id(lat_and_lon_global).length() ) );
            id(auto_location_found).publish_state("Using preprogrammed location");
            ESP_LOGD("onBoot", "Using preprogrammed location");
        } else if (id(lat_global) == 40.669010f && id(lon_global) == -73.942871f) {
            // default preprogrammed location hasn't been updated let's auto detect the location
            MelachaPlug::autoSetLocation();
            ESP_LOGD("onBoot", "Using auto location");
        } else {
            // otherwise use the global variable we have set
            id(lat_text_sensor).publish_state( to_string( id(lat_global) ) );
            id(lon_text_sensor).publish_state( to_string( id(lon_global) ) );
            id(auto_location_found).publish_state("Using saved location");
            ESP_LOGD("onBoot", "Using saved location");
        }
    }

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

        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == false) {
            // since its a weeekday calculate hebrew day by Shabbos start time
            MelachaPlug::hdateAddDay();  // advance hebrew date
            MelachaPlug::checkForIssurMelacha();
            MelachaPlug::publishHebrewDay();
            MelachaPlug::updateTextSensors();
        } else {
            ESP_LOGD("onShabbosStart", "it's Shabbos or Yom Tov, wating for later zman to advance hebrew date");
        }
    }

    void onShabbosEnd() {
        // gets triggered every day at the potential Shabbos or Yom Tov end time

        // check if melacha is permitted today
        if (isassurbemelachah(hdate) == true) {
            // since its a Shabbos or Yom Tov calculate hebrew day by Shabbos end time
            MelachaPlug::hdateAddDay();  // advance hebrew date
            MelachaPlug::checkForIssurMelacha();
            MelachaPlug::publishHebrewDay();
            MelachaPlug::updateTextSensors();
        } else {
            ESP_LOGD("onShabbosStart", "it's a weekday, date was already advanced");
        }
    }

    void onLatitudeSet(std::string x) {
        ESP_LOGD("onLatitudeSet", "onLatitudeSet ran");
        id(mysun).set_latitude(atof(x.c_str()));
        id(lat_global) = atof(x.c_str());
        id(sntp_time).update(); // will update hebrew date and check date
    }

    void onLongitudeSet(std::string x) {
        ESP_LOGD("onLongitudeSet", "onLongitudeSet ran");
        id(mysun).set_longitude(atof(x.c_str()));
        id(lon_global) = atof(x.c_str());
        id(sntp_time).update(); // will update hebrew date and check date
    }

    void onKeyboardSelect(std::string x) {
        if (x == "BACKSPACE") {
            if (!id(keyboard).state.empty()) {
            std::string update = id(keyboard).state;
            update.pop_back();
            id(keyboard).publish_state(update);
            }
        } else if(x == "CLEAR") {
            id(keyboard).publish_state("");
        } else if(x == "Repeat last character") {
            std::string update = id(keyboard).state;
            update.push_back( update.back() );
            id(keyboard).publish_state(update);
        } else if(x == "Save to Latitude") {
            id(lat_text_sensor).publish_state(id(keyboard).state);
            id(keyboard).publish_state("");
            id(auto_location_found).publish_state("Manual location entered");
        } else if(x == "Save to Longitude") {
            id(lon_text_sensor).publish_state(id(keyboard).state);
            id(keyboard).publish_state("");
            id(auto_location_found).publish_state("Manual location entered");
        } else {
            id(keyboard).publish_state(id(keyboard).state + x.c_str());
        }
    }

} // namespace MelachaPlug

// ----------- test