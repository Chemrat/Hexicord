#ifndef HEXICORD_TYPES_EMBED_HPP
#define HEXICORD_TYPES_EMBED_HPP

#include <string>
#include <boost/optional/optional.hpp>
#include "hexicord/json.hpp"
#include "hexicord/types/nlohmann_helper.hpp"

namespace Hexicord {
    struct Embed {
        struct Thumbnail {
            Thumbnail() = default;
            inline Thumbnail(const std::string& url) : url(url) {}
            Thumbnail(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Source url of thumbnail (only supports http(s) and attachments).
            std::string url;

            /// Proxied url of the image. Should not be set by user.
            std::string proxyUrl;

            /// Thumbnail deminisions. Should not be set by user.
            unsigned height = 0, width = 0;
        };

        struct Video {
            Video() = default;
            inline Video(const std::string& url) : url(url) {}
            Video(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Source url of video.
            std::string url;

            /// Image deminisions. Should not be set by user.
            unsigned height = 0, width = 0;
        };

        struct Image {
            Image() = default;
            inline Image(const std::string& url) : url(url) {}
            Image(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Source url of image (only supports http(s) and attachments).
            std::string url;

            /// Proxied url of the image. Should not be set by user.
            std::string proxyUrl = "";

            /// Image dimensions. Should not be set by user.
            unsigned height = 0, width = 0;
        };

        struct Provider {
            Provider() = default;
            inline Provider(const std::string& name, const std::string& url = "") : name(name), url(url) {}
            Provider(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Name of provider.
            std::string name;

            /// Url of provider.
            std::string url;
        };

        struct Author {
            Author() = default;
            inline Author(const std::string& name,
                          const std::string& url = "",
                          const std::string& iconUrl = "") : name(name), url(url), iconUrl(iconUrl) {}
            Author(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Name of author.
            std::string name;

            /// Url of author.
            std::string url;

            /// Url of author icon (only supports http(s) and attachments).
            std::string iconUrl = "";

            /// A proxied url of author icon. Should not be set by user.
            std::string proxyIconUrl = "";
        };

        struct Footer {
            Footer() = default;
            inline Footer(const std::string& text, const std::string& iconUrl) : text(text), iconUrl(iconUrl) {}
            Footer(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Footer text.
            std::string text;

            /// Url of footer icon (only supports http(s) and attachments).
            std::string iconUrl;

            /// A proxied url of footer icon. Should not be set by user.
            std::string proxyIconUrl = "";
        };

        struct Field {
            Field() = default;
            Field(const std::string& name, const std::string& value, bool inline_ = false)
                : name(name), value(value), inline_(inline_) {}
            Field(const nlohmann::json& json);
            nlohmann::json dump() const;

            /// Name of the field.
            std::string name;

            /// Value of the field.
            std::string value;

            /// Whether or not this field should display inline.
            bool inline_ = false;
        };

        Embed() = default;
        Embed(const nlohmann::json& json);
        nlohmann::json dump() const;

        /// Title of embed.
        std::string title;

        /// Description of embed.
        std::string description;

        /// Url of embed. Empty if none.
        std::string url;

        /// ISO 8601 timestamp of embed content. Empty if none.
        std::string timestamp;

        /// Color code of the embed. 0 if none.
        unsigned color;

        /// Footer information.
        boost::optional<Footer> footer;

        /// Image information.
        boost::optional<Image> image;

        /// Thumbnail information.
        boost::optional<Thumbnail> thumbnail;

        /// Video information.
        boost::optional<Video> video;

        /// Provider information.
        boost::optional<Provider> provider;

        /// Author information.
        boost::optional<Author> author;

        /// Fields information if any.
        std::vector<Field> fields;
    };
} // namespace Hexicord

#endif // HEXICORD_TYPES_EMBED_HPP
