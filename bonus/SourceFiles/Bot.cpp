#include "../HeaderFiles/Server.hpp"

#include <curl/curl.h>
#include <sstream>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string getWeather(const std::string& city) {
    std::string apiKey = "6062e89453f4af76f7635ad3fafa8a78";
    std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";
    CURL* curl;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return readBuffer;
}

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
    std::stringstream ss(message.substr(message.find(" ")));
    std::string city, commad, channelName;
    ss >> channelName >> commad >> city;
    if (city.empty() || channelName.empty())
    {
        ERR_BOTCALLED(client, (channelName.empty() ? "" : channelName), "Could not fetch weather data. Check the city name or API key.", isNickTaken(channelName));
        return false;
    }
    std::string response = getWeather(city);
    std::string cityName = parseJSONValue(response, "name");
    std::string temp = parseJSONValue(response, "temp");
    std::string weatherDescription = parseJSONValue(response, "description");

    std::stringstream weatherMsg;
    if (!cityName.empty() && !temp.empty() && !weatherDescription.empty()) {
        weatherMsg << "Weather in " << cityName << ": " << weatherDescription
                  << " with " << temp << "°C.";
        RPL_BOTCALLED(client, channelName, weatherMsg, isNickTaken(channelName));
    } else {
        ERR_BOTCALLED(client, channelName, "Could not fetch weather data. Check the city name or API key.", isNickTaken(channelName));
    }
    return true;
}

void Server::RPL_BOTCALLED(Client &client, const std::string &channelName, std::stringstream &Weather, bool isClient) {
    std::stringstream ss;

    Client *tmp = &client;
    if (isClient) {
        tmp = getClientByNick(channelName);
    }
    ss << ":" << "<WeatherBot>"
        << ((isClient) ? " PRIVMSG " : " 801 ") << channelName
        << " :"<< Weather.str() << "\r\n";
    send(tmp->getSocket(), ss.str().c_str(), ss.str().length(), 0);
}

void Server::ERR_BOTCALLED(Client &client, const std::string &channelName,const std::string &Weather, bool isClient) {
    std::stringstream ss;

    Client *tmp = &client;
    if (isClient) {
        tmp = getClientByNick(channelName);
    }
    ss << ":" << "<WeatherBot>"
        << ((isClient) ? " PRIVMSG " : " 802 ") << channelName
        << " :"<< Weather << "\r\n";
    send(tmp->getSocket(), ss.str().c_str(), ss.str().length(), 0);
}
