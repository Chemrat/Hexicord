#include <hexicord/internal/rest.hpp>

#include <locale>                                   // std::tolower, std::locale
#include <boost/asio/ssl/rfc2818_verification.hpp>  // boost::asio::ssl::rfc2818_verification.hpp
#include <boost/asio/connect.hpp>                   // boost::asio::connect
#include <boost/beast/http/write.hpp>               // boost::beast::http::write
#include <boost/beast/http/read.hpp>                // boost::beast::http::read
#include <boost/beast/http/vector_body.hpp>         // boost::beast::http::vecor_body
#include <boost/beast/core/flat_buffer.hpp>         // boost::beast::flat_buffer
#include <hexicord/internal/utils.hpp>              // Utils::randomAsciiString

namespace ssl = boost::asio::ssl;
using     tcp = boost::asio::ip::tcp;

namespace Hexicord { namespace REST {
namespace _detail {
    std::string stringToLower(const std::string& input) {
        std::string result;
        result.reserve(input.size());

        for (char ch : input) {
            result.push_back(std::tolower(ch, std::locale("C")));
        }
        return result;
    }
}

HTTPSConnection::HTTPSConnection(boost::asio::io_service& ioService, const std::string& serverName) 
    : serverName(serverName)
    , tlsctx(boost::asio::ssl::context::tlsv12_client)
    , stream(ioService, tlsctx) {


    tlsctx.set_verify_mode(ssl::verify_peer | ssl::verify_fail_if_no_peer_cert);
    tlsctx.set_verify_callback(ssl::rfc2818_verification(serverName));
    tlsctx.set_default_verify_paths();
}

void HTTPSConnection::open() {
    tcp::resolver resolver(stream.get_io_service());
    resolutionResult = resolver.resolve({ serverName, "https" });

    boost::asio::connect(stream.next_layer(), resolutionResult);
    stream.next_layer().set_option(tcp::no_delay(true));
    stream.handshake(ssl::stream_base::client);
    alive = true;
}

void HTTPSConnection::close() {
    boost::system::error_code ec;
    stream.shutdown(ec);
    if (ec && 
        ec != boost::asio::error::eof && 
        ec != boost::asio::ssl::error::stream_truncated &&
        ec != boost::asio::error::broken_pipe &&
        ec != boost::asio::error::connection_reset) {

        throw boost::system::system_error(ec);
    }
    stream.next_layer().close();
    alive = false;
}

bool HTTPSConnection::isOpen() const {
    return stream.lowest_layer().is_open() && alive;
}

HTTPResponse HTTPSConnection::request(const HTTPRequest& request) {
    //
    // Prepare request
    //
    boost::beast::http::request<boost::beast::http::vector_body<uint8_t> > rawRequest;
    
    rawRequest.method_string(request.method);
    rawRequest.target(request.path);
    rawRequest.version = request.version;

    // Set default headers. 
    rawRequest.set("User-Agent", "Generic HTTP 1.1 Client");
    rawRequest.set("Connection", "keep-alive");
    rawRequest.set("Accept",     "*/*");
    rawRequest.set("Host",       serverName);
    if (!request.body.empty()) {
        rawRequest.set("Content-Length", std::to_string(request.body.size()));
        rawRequest.set("Content-Type",   "application/octet-stream");
    }

    // Set per-connection headers.
    for (const auto& header : connectionHeaders) {
        rawRequest.set(header.first, header.second);
    }

    // Set per-request
    for (const auto& header : request.headers) {
        rawRequest.set(header.first, header.second);
    }

    if (!request.body.empty()) {
        rawRequest.body = request.body;
    }

    //
    // Perform request.
    //
    
    boost::system::error_code ec;

    rawRequest.prepare_payload();
    alive = false;
    boost::beast::http::write(stream, rawRequest, ec);
    if (ec && ec != boost::beast::http::error::end_of_stream) throw boost::system::system_error(ec);

    boost::beast::http::response<boost::beast::http::vector_body<uint8_t> > response;
    boost::beast::flat_buffer buffer;
    boost::beast::http::read(stream, buffer, response);

    REST::HTTPResponse responseStruct;
    responseStruct.statusCode = response.result_int();
    responseStruct.body      = response.body;
    for (const auto& header : response) {
        responseStruct.headers.insert({ header.name_string().to_string(), header.value().to_string() });
    }
    alive = (response["Connection"].to_string() != "close");
    
    return responseStruct;
}

HTTPRequest buildMultipartRequest(const std::vector<MultipartEntity>& elements) {
    HTTPRequest request;
    std::ostringstream oss;

    // XXX: There is some reason for 400 Bad Request coming from Utils::randomAsciiString.
    // std::string boundary = Utils::randomAsciiString(64);
    std::string boundary = "LPN3rnFZYl77S6RI2YHlqA1O1NbvBDelp1lOlMgjSm9VaOV7ufw5fh3qvy2JUq";

    request.headers["Content-Type"] = std::string("multipart/form-data; boundary=") + boundary;

    for (auto it = elements.begin(); it != elements.end(); ++it) {
        const auto& element = *it;

        oss << "--" << boundary << "\r\n"
            << "Content-Disposition: form-data; name=\"" << element.name << "\"";
        if (!element.filename.empty()) {
            oss << "; filename=\"" << element.filename << '"';
        }
        oss << "\r\n";
        for (const auto& header : element.additionalHeaders) {
            oss << header.first << ": " << header.second << "\r\n";
        }
        oss << "\r\n";
        for (uint8_t byte : element.body) {
            oss << byte;
        }
        oss << "\r\n";

        oss << "--" << boundary;
        if (it == elements.end() - 1) {
            oss << "--";
        }
        oss << "\r\n";
    }

    std::string resultStr = oss.str();
    request.body = std::vector<uint8_t>(resultStr.begin(), resultStr.end());
    return request;
}

}} // namespace Hexicord::REST
