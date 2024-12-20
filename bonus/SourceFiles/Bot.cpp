#include "../HeaderFiles/Server.hpp"

std::string getWeather(const std::string& city) {
    std::string apiKey = "6062e89453f4af76f7635ad3fafa8a78";
    std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";
    std::string command = "curl -s \"" + url + "\" -o weather.json";
    system(command.c_str());
    std::ifstream file("weather.json");
    if (!file.is_open())
        return "Error: Unable to open weather.json";
    std::string response((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    remove("weather.json");
    if (response.find("error") != std::string::npos)
        return "City unknown";
    return response;
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
