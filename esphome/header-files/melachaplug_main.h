/****
Copyright (c) 2023 RebbePod

This library is free software; you can redistribute it and/or modify it 
under the terms of the GNU Lesser GeneralPublic License as published by the Free Software Foundation; 
either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; 
without even the impliedwarranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
See the GNU Lesser General Public License for more details. 
You should have received a copy of the GNU Lesser General Public License along with this library; 
if not, write tothe Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA, 
or connect to: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
****/

#pragma once

#include "../../../../header-files/hebrewcalender_melachaplug.h"
#include "esphome.h"
using namespace esphome;
using namespace time;

namespace Hdate  {
    hdate current_hdate;  // global varible for todays hebrew date
} // namespace Hdate

namespace MelachaPlug  {

    time_t getPlagTime() {
        ESP_LOGD("getPlagTime", "getPlagTime ran");
        time::ESPTime date = id(sntp_time).now();
        date.hour = date.minute = date.second = 0;                          // uptime time to 00:00:00
        date.recalc_timestamp_utc();
        auto sunrise_today = id(mysun).sunrise(date, -1.583);               // time of hanetz amiti Alter Rebbe
        auto sunset_today = id(mysun).sunset(date, -1.583);                 // time of shkiah amitis Alter Rebbe
        float shaahzmanis = (sunset_today->timestamp - sunrise_today->timestamp) / 12;
        time_t plag = sunrise_today->timestamp + (shaahzmanis * 10.75);     // time of plag Alter Rebbe

        ESP_LOGD("getPlagTime", "sunrise_today (amiti): %lu", sunrise_today->timestamp);
        ESP_LOGD("getPlagTime", "sunset_today (amiti): %lu", sunset_today->timestamp);
        ESP_LOGD("getPlagTime", sunrise_today->strftime("sunrise_today: --- %a, %b %e, %Y, %l:%M %p ---").c_str());
        ESP_LOGD("getPlagTime", sunset_today->strftime("sunset_today: --- %a, %b %e, %Y, %l:%M %p ---").c_str());
        ESP_LOGD("getPlagTime", "shaahzmanis: %f", shaahzmanis);
        ESP_LOGD("getPlagTime", "plag timestamp: %lu", plag);
        return plag;
    }

    void setEretzYisrael() {
        if (id(eretz_yisrael).state == true) {
            setEY(&Hdate::current_hdate, true);
            ESP_LOGD("setEretzYisrael", "EY set to true");
        } else {
            setEY(&Hdate::current_hdate, false);
            ESP_LOGD("setEretzYisrael", "EY set to false");
        }
    }

    void updateHdate() {
        // only run once we have the proper time and location
        ESP_LOGD("updateHdate", "updateHdate ran");
        if (id(time_valid) == false) {
            ESP_LOGD("updateHdate", "date not valid");
            return;
        }
        float elevation_deg;
        int offset_minutes;
        time_t timestamp_sunset;

        time::ESPTime date = id(sntp_time).now();                    // we use this variable later
        time_t timestamp_now = date.timestamp;                       // save the current timestamp before updating the date
        struct tm tm_time_now = date.to_c_tm();
        Hdate::current_hdate = convertDate(tm_time_now);             // convert current english date to the hebrew date
        MelachaPlug::setEretzYisrael();                              // confirm EY is set correctly

        // check which degree and offset we calculate from
        if (isassurbemelachah(Hdate::current_hdate) == false) {
            ESP_LOGD("updateHdate", "since its a weeekday, we calculate hebrew day starting from Shabbos start time");
            elevation_deg = id(deg_shabbos_starts).state; 
            offset_minutes = id(min_offset_start).state;   // use the start time offset
        } else {
            ESP_LOGD("updateHdate", "since its Shabbos or Yom Tov, we calculate hebrew day starting from Shabbos end time");
            elevation_deg = id(deg_shabbos_ends).state;
            offset_minutes = id(min_offset_end).state;   // use the end time offset
        }

        // check if using early shabbos and use plag as start time
        if (id(early_shabbos).state == true && isassurbemelachah(Hdate::current_hdate) == false) {
            ESP_LOGD("updateHdate", "Early Shabbos Switch is ON use plag as the start time");
            timestamp_sunset = getPlagTime();
        } else {
            ESP_LOGD("updateHdate", "using elevation_deg to calculate shabbos start/end time");
            date.hour = date.minute = date.second = 0;                          // uptime time to 00:00:00
            date.recalc_timestamp_utc();
            auto sunset_today = id(mysun).sunset(date, elevation_deg);          // get sunset time at specified degree
            ESP_LOGD("updateHdate", sunset_today->strftime("sunset_today: --- %a, %b %e, %Y, %l:%M %p ---").c_str());
            timestamp_sunset = sunset_today->timestamp;
        }
        ESP_LOGD("updateHdate", "timestamp_now: %lu", timestamp_now);
        ESP_LOGD("updateHdate", "timestamp_sunset before offset applied: %lu", timestamp_sunset);
        timestamp_sunset += offset_minutes * 60;  // apply minute offset to sunset
        ESP_LOGD("updateHdate", "timestamp_sunset after offset applied: %lu", timestamp_sunset);

        if (timestamp_now > timestamp_sunset) {
            ESP_LOGD("updateHdate", "it's between midnight and sunset adding a day to Hdate");
            hdateaddday(&Hdate::current_hdate, 1);      // advance hdate
        } else {
            ESP_LOGD("updateHdate", "it's not between midnight and sunset");
        }
        id(hdate_valid) = true;    // let the global varibale know that we updated the date

        date = date.from_epoch_local(timestamp_sunset);
        ESP_LOGD("updateHdate", "timestamp_sunset: %lu", date.timestamp);
        id(todays_melacha_check_time).publish_state( date.strftime("%a, %b %e, %l:%M %p") );
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
            ESP_LOGD("publishHebrewDay", "Month not found in hebrew date!");
        }

        id(hebrew_date).publish_state( to_string(Hdate::current_hdate.day) + " " + hebrew_month + " " + to_string(Hdate::current_hdate.year) );
        ESP_LOGD("publishHebrewDay", "Year: %d Month: %s Day: %d", Hdate::current_hdate.year, hebrew_month.c_str(), Hdate::current_hdate.day);
    }

    void setAlterRebbeZmanim() {
        // reverts to default Alter Rebbe zmanin
        id(deg_shabbos_starts).publish_state(-0.833);
        id(deg_shabbos_ends).publish_state(-8.5);
    }

    void updateTextSensors() {
        id(ts_sunset).update();
        id(ts_tzeit).update();
    }

    void setShabbosMode() {
        // should only be run once were are certain the hebrew date is correct
        ESP_LOGD("setShabbosMode", "setShabbosMode ran");
        if (isassurbemelachah(Hdate::current_hdate) == false) {
            ESP_LOGD("setShabbosMode", "melacha permitted");
            // weekday, turn off Shabbos mode
            if (id(override_shabbos_mode).state == false) {
                id(shabbos_mode).turn_off(); // turn off Shabbos mode
                ESP_LOGD("setShabbosMode", "turned off Shabbos Mode");
            }
        } else {
            ESP_LOGD("setShabbosMode", "melacha prohibited");
            // Shabbos or Yom Tov, turn on Shabbos mode
            if (id(override_shabbos_mode).state == false) {
                id(shabbos_mode).turn_on(); // turn on Shabbos mode
                ESP_LOGD("setShabbosMode", "turned on Shabbos Mode");
            }
        }
    }

    // ---------------------------------------- On Sequences ----------------------------------------
    void runMelachaChecks() {
        ESP_LOGD("runMelachaChecks", "runMelachaChecks ran");
        MelachaPlug::updateHdate();           // update the hebrew date
        MelachaPlug::setShabbosMode();        // checks if issur and turns on Shabbos mode
        MelachaPlug::publishHebrewDay();      // update the hebrew day text sensor
        MelachaPlug::updateTextSensors();
    }
} // namespace MelachaPlug


namespace LocationExtras  {

    void detectLocation() {
        WiFiClient wifiClient;
        HTTPClient http;
        http.begin(wifiClient, "http://ip-api.com/json/?fields=status,regionName,city,zip,lat,lon"); // API from http://ip-api.com/
        int httpResponseCode = http.GET();
        std::string payload = http.getString().c_str();
        ESP_LOGD("detectLocation", "Curernt Location: %s", payload.c_str());
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

            ESP_LOGD("detectLocation", "status: %s, state: %s, city: %s, zip: %s, lat: %s, lon: %s", status.c_str(), state.c_str(), city.c_str(), zip.c_str(), lat.c_str(), lon.c_str());
            id(latitude_number).publish_state(atof(lat.c_str()));
            id(longitude_number).publish_state(atof(lon.c_str()));
            id(location_info).publish_state("Auto detected: " + city + ", " + state + " " + zip);
        } else {
            id(location_info).publish_state("Failed to get your location");
            ESP_LOGD("detectLocation", "Failed to get your location");
        }
    }

    void revertToDefaultLocation(double latitude, double longitude) {
        // reverts to default location
        ESP_LOGD("revertToDefaultLocation", "latitude: %f longitude: %f", latitude, longitude);
        if(latitude == 40.669000) {
            // same as default 770 location. we change some digits so it shouldn't
            // trigger the auto location detection feature. See LocationExtras::onBoot()
            ESP_LOGD("revertToDefaultLocation", "same as default 770 location");
            id(latitude_number).publish_state(40.668888f);
            id(location_info).publish_state("Using 770 location");
        } else {
            ESP_LOGD("revertToDefaultLocation", "using exact coded latitude");
            id(latitude_number).publish_state(latitude);
            id(location_info).publish_state("Using coded location");
        }
        id(longitude_number).publish_state(longitude);
    }

    // ---------------------------------------- On Sequences ----------------------------------------

    void onBoot(double latitude, double longitude) {
        ESP_LOGD("onBoot", "--------- sun elevation: %f", id(mysun).elevation() );
        ESP_LOGD("onBoot", "lat: %f lon: %f", id(latitude_number).state, id(latitude_number).state );
        ESP_LOGD("onBoot", "latitude: %f longitude: %f", latitude, longitude);

        if (id(latitude_number).state == 40.669000 && id(longitude_number).state == -73.942770) {
            // default coded location hasn't been updated, let's auto detect the location
            LocationExtras::detectLocation();
            ESP_LOGD("onBoot", "Using auto location");
        } else {
            // otherwise use the number variable we have set
            if (id(latitude_number).state == latitude && id(longitude_number).state == longitude) {
                id(location_info).publish_state("Using coded location");
                ESP_LOGD("onBoot", "Using coded location");
            } else {
                id(location_info).publish_state("Using saved location");
                ESP_LOGD("onBoot", "Using saved location");
            }
        }
    }

    void onLatitudeSet() {
        ESP_LOGD("onLatitudeSet", "onLatitudeSet ran");
        id(mysun).set_latitude(id(latitude_number).state);
        id(sntp_time).update(); // will also update hebrew date and check date
    }

    void onLongitudeSet() {
        ESP_LOGD("onLongitudeSet", "onLongitudeSet ran");
        id(mysun).set_longitude(id(longitude_number).state);
        id(sntp_time).update(); // will also update hebrew date and check date
    }
} // namespace LocationExtras