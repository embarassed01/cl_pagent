#include "json/reader.h"
#include <stdio.h>
#include <iostream>
#include <json/json.h>

int main() {
    printf("test jsoncpp\n");

    constexpr bool shouldUseOldWay = false;
    // write to string
    Json::Value root;
    Json::Value data;
    root["action"] = "run";
    data["number"] = 1;
    root["data"] = data;
    if (shouldUseOldWay) {
        Json::FastWriter writer;
        const std::string json_file = writer.write(root);
        std::cout << json_file << std::endl;
    } else {
        Json::StreamWriterBuilder builder;
        const std::string json_file = Json::writeString(builder, root);
        std::cout << json_file << std::endl;
    }

    // read from string
    const std::string rawJson = R"({"Age": 20, "Name": "colin"})";
    const auto rawJsonLength = static_cast<int>(rawJson.length());
    JSONCPP_STRING err;
    Json::Value root2;
    if (shouldUseOldWay) {
        Json::Reader reader;
        reader.parse(rawJson, root2);
    } else {
        Json::CharReaderBuilder builder;
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJsonLength, &root2, &err)) {
            std::cout << "error" << std::endl;
            return EXIT_FAILURE;
        }
    }
    const std::string name = root2["Name"].asString();
    const int age = root2["Age"].asInt();
    std::cout << name << std::endl;
    std::cout << age << std::endl;

    return 0;
}