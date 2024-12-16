#include "../HeaderFiles/Server.hpp"

#include <curl/curl.h>
#include <sstream>

// Callback to handle the API response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch weather data
std::string getWeather(const std::string& city) {
    std::string apiKey = std::getenv("API_KEY");
    std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return readBuffer;
}

// Function to parse a value from JSON manually
std::string parseJSONValue(const std::string& json, const std::string& key) {
    size_t keyPos = json.find("\"" + key + "\":");
    if (keyPos == std::string::npos) {
        return "";
    }
    size_t start = json.find(":", keyPos) + 1;
    size_t end;
    if (json[start] == '"') {
        start++;
        end = json.find('"', start);
    } else {
        end = json.find(',', start);
        if (end == std::string::npos) {
            end = json.find('}', start);
        }
    }
    return json.substr(start, end - start);
}

bool Server::processBotcommand(Client &client, const std::string &message) {
    if (message.find("!Weather") == std::string::npos)
        return false;
    std::stringstream ss(message.substr(message.find("#")));
    std::string city, commad, channelName;
    ss >> channelName >> commad >> city;
    std::string response = getWeather(city);
    std::string cityName = parseJSONValue(response, "name");
    std::string temp = parseJSONValue(response, "temp");
    std::string weatherDescription = parseJSONValue(response, "description");

    // Display the result
    if (city.empty() || channelName.empty())
    {
        client.ERR_BOTCALLED(client, "#chat", "Could not fetch weather data. Check the city name or API key.");
        return true;
    }
    std::stringstream weatherMsg;
    if (!cityName.empty() && !temp.empty() && !weatherDescription.empty()) {
        weatherMsg << "Weather in " << cityName << ": " << weatherDescription
                  << " with " << temp << "°C.";
        client.RPL_BOTCALLED(client, "#chat", weatherMsg);
    } else {
        client.ERR_BOTCALLED(client, "#chat", "Could not fetch weather data. Check the city name or API key.");
    }
    return true;
}