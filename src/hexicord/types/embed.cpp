#include "hexicord/types/embed.hpp"

namespace Hexicord {
    Embed::Thumbnail::Thumbnail(const nlohmann::json& json)
        : url       (json.at("url")       .get<std::string>())
        , proxyUrl  (json.value("proxy_url", ""))
        , height    (json.value("height", 0))
        , width     (json.value("width", 0)) {}

    nlohmann::json Embed::Thumbnail::dump() const {
        nlohmann::json result = {
            { "url", url }
        };

        if (!proxyUrl.empty()) result["proxy_url"] = proxyUrl;
        if (height != 0)       result["height"]    = height;
        if (width != 0)        result["width"]     = width;

        return result;
    }

    Embed::Video::Video(const nlohmann::json& json)
        : url    (json.at("url").get<std::string>())
        , height (json.value("height", 0))
        , width  (json.value("width", 0)) {}

    nlohmann::json Embed::Video::dump() const {
        nlohmann::json result = {
            { "url", url }
        };

        if (height != 0) result["height"]  = height;
        if (width != 0)  result["width"]   = width;

        return result;
    }

    Embed::Image::Image(const nlohmann::json& json)
        : url       (json.at("url").get<std::string>())
        , proxyUrl  (json.value("proxy_url", ""))
        , height    (json.value("height", 0))
        , width     (json.value("width", 0)) {}

    nlohmann::json Embed::Image::dump() const {
        nlohmann::json result = {
            { "url", url },
        };

        if (!proxyUrl.empty()) result["proxy_url"] = proxyUrl;
        if (height != 0)       result["height"]    = height;
        if (width != 0)        result["width"]     = width;

        return result;
    }

    Embed::Provider::Provider(const nlohmann::json& json)
        : name (json.at("name").get<std::string>())
        , url  (json.at("url") .get<std::string>()) {}

    nlohmann::json Embed::Provider::dump() const {
        return {
            { "name", name },
            { "url",  url  }
        };
    }

    Embed::Author::Author(const nlohmann::json& json)
        : name         (json.at("name").get<std::string>())
        , url          (json.value("url",       ""))
        , iconUrl      (json.value("icon_url",  ""))
        , proxyIconUrl (json.value("proxy_url", "")) {}

    nlohmann::json Embed::Author::dump() const {
        nlohmann::json result = {
            { "name", name }
        };

        if (!url.empty())          result["url"] = url;
        if (!iconUrl.empty())      result["icon_url"] = iconUrl;
        if (!proxyIconUrl.empty()) result["proxy_url"] = proxyIconUrl;

        return result;
    }

    Embed::Footer::Footer(const nlohmann::json& json)
        : text          (json.at("text").get<std::string>())
        , iconUrl       (json.value("icon_url", ""))
        , proxyIconUrl  (json.value("proxy_url", "")) {}

    nlohmann::json Embed::Footer::dump() const {
        nlohmann::json result {
            { "text", text },
        };

        if (!iconUrl.empty())      result["icon_url"]  = iconUrl;
        if (!proxyIconUrl.empty()) result["proxy_url"] = proxyIconUrl;

        return result;
    }

    Embed::Field::Field(const nlohmann::json& json)
        : name    (json.at("name")  .get<std::string>())
        , value   (json.at("value") .get<std::string>())
        , inline_ (json.at("inline").get<bool>()       ) {}

    nlohmann::json Embed::Field::dump() const {
        return {
            { "name",   name    },
            { "value",  value   },
            { "inline", inline_ }
        };
    }

    Embed::Embed(const nlohmann::json& json)
        : title         (json.value("title",        ""))
        , description   (json.value("description",  ""))
        , url           (json.value("url",          ""))
        , timestamp     (json.value("timestamp",    ""))
        , color         (json.value("color",        0 )) {

            if (json.count("footer")    != 0) footer    = Footer(json["footer"]);
            if (json.count("image")     != 0) image     = Image(json["image"]);
            if (json.count("thumbnail") != 0) thumbnail = Thumbnail(json["thumbnail"]);
            if (json.count("video")     != 0) video     = Video(json["video"]);
            if (json.count("provider")  != 0) provider  = Provider(json["provider"]);
            if (json.count("author")    != 0) author    = Author(json["author"]);

            if (json.count("fields")    != 0 &&
                json["fields"].size()   != 0) fields    = json["fields"].get<std::vector<Field>>();
    }

    nlohmann::json Embed::dump() const {
        nlohmann::json result;

        if (!title.empty())         result["title"] = title;
        if (!description.empty())   result["description"] = description;
        if (!url.empty())           result["url"] = url;
        if (!timestamp.empty())     result["timestamp"] = timestamp;
        if (color != 0)             result["color"] = color;

        if (footer)     result["footer"]    = *footer;
        if (image)      result["image"]     = *image;
        if (thumbnail)  result["thumbnail"] = *thumbnail;
        if (video)      result["video"]     = *video;
        if (provider)   result["provider"]  = *provider;
        if (author)     result["author"]    = *author;

        if (!fields.empty()) result["fields"] = fields;

        return result;
    }


} // namespace Hexicord
