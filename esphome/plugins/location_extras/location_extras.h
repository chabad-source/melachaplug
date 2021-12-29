#pragma once

#include "esphome.h"
using namespace esphome;
using namespace time;

namespace LocationExtras  {

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
            id(lat_text_sensor).publish_state( id(lat_and_lon_global).substr(0, separator) );
            id(lon_text_sensor).publish_state( id(lat_and_lon_global).substr( separator +1, id(lat_and_lon_global).length() ) );
            id(auto_location_found).publish_state("Using preprogrammed location");
            ESP_LOGD("onBoot", "Using preprogrammed location");
        } else if (id(lat_global) == 40.669010f && id(lon_global) == -73.942871f) {
            // default preprogrammed location hasn't been updated let's auto detect the location
            LocationExtras::autoSetLocation();
            ESP_LOGD("onBoot", "Using auto location");
        } else {
            // otherwise use the global variable we have set
            id(lat_text_sensor).publish_state( to_string( id(lat_global) ) );
            id(lon_text_sensor).publish_state( to_string( id(lon_global) ) );
            id(auto_location_found).publish_state("Using saved location");
            ESP_LOGD("onBoot", "Using saved location");
        }
    }

    void onLatitudeSet(std::string x) {
        ESP_LOGD("onLatitudeSet", "onLatitudeSet ran");
        id(mysun).set_latitude(atof(x.c_str()));
        id(lat_global) = atof(x.c_str());
        id(sntp_time).update(); // will also update hebrew date and check date
    }

    void onLongitudeSet(std::string x) {
        ESP_LOGD("onLongitudeSet", "onLongitudeSet ran");
        id(mysun).set_longitude(atof(x.c_str()));
        id(lon_global) = atof(x.c_str());
        id(sntp_time).update(); // will also update hebrew date and check date
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
} // namespace LocationExtras