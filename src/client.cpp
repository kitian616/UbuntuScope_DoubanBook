#include <client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>


namespace http = core::net::http;
namespace net = core::net;

using namespace std;

Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}


void Client::get(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot, path, parameters);
    configuration.uri = client->uri_to_string(uri);

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        // Open weather map API error code can either be a string or int
        QVariant cod = root.toVariant().toMap()["cod"];
        if ((cod.canConvert<QString>() && cod.toString() != "200")
                || (cod.canConvert<unsigned int>() && cod.toUInt() != 200)) {
            throw domain_error(root.toVariant().toMap()["message"].toString().toStdString());
        }
    } catch (net::Error &) {
    }
}

Client::BookRes Client:: get_booklist(const QVariantMap& variant){
    BookRes result;

    QVariantList book_list = variant["books"].toList();
    for (const QVariant &book : book_list) {
        QVariantMap book_map = book.toMap();
        //
        Authors authors;
        QVariantList author_list = book_map["author"].toList();
        for (const QVariant &author : author_list) {
            std::string author_str = author.toString().toStdString();
            Author author_item = {author_str};
            authors.emplace_back(author_item);
        }

        QVariantMap rating_map = book_map["rating"].toMap();
        Book_rating rating = {
            rating_map["numRaters"].toInt(),
            rating_map["average"].toDouble(),
        };

        Tags tags;
        QVariantList tag_list = book_map["tags"].toList();
        for(const QVariant &tag : tag_list) {
            QVariantMap tag_map = tag.toMap();
            Book_tag tag_item = {
                tag_map["count"].toInt(),
                tag_map["name"].toString().toStdString(),
                tag_map["title"].toString().toStdString(),
            };
            tags.emplace_back(tag_item);
        }
        QVariantMap image_map = book_map["images"].toMap();
        Book_avatar avatar_url = {
            image_map["small"].toString().toStdString(),
            image_map["large"].toString().toStdString(),
            image_map["medium"].toString().toStdString(),
        };
        // We add each result to our list
        result.booklist.emplace_back(
            Book {
                book_map["id"].toString().toStdString(),
                book_map["title"].toString().toStdString(),
                rating,
                authors,
                book_map["publisher"].toString().toStdString(),
                tags,
                book_map["pubdate"].toString().toStdString(),
                book_map["origin_title"].toString().toStdString(),
                avatar_url,
                book_map["url"].toString().toStdString(),
                book_map["catalog"].toString().toStdString(),
            }
        );
    }
    return result;
}

Client::BookRes Client:: search_books_by_name(const std::string &query){
    QJsonDocument root;

    // Build a URI and get the contents.
    // The fist parameter forms the path part of the URI.
    // The second parameter forms the CGI parameters.
    get( { "book/search"}, { { "apikey", "0f4b40e701d84e822de1c2b50588eb86" }, { "q", query } }, root);
    // https://api.soundcloud.com/tracks.json?client_id=apigee&q=<query>
    // https://api.douban.com/v2/book/search?q=<query>&apikey=0f4b40e701d84e822de1c2b50588eb86
    QVariantMap variant = root.toVariant().toMap();
    return get_booklist(variant);
}

Client::BookRes Client:: search_books_by_tag(const std::string &query){
    QJsonDocument root;

    // Build a URI and get the contents.
    // The fist parameter forms the path part of the URI.
    // The second parameter forms the CGI parameters.
    get( { "book/search"}, { { "apikey", "0f4b40e701d84e822de1c2b50588eb86" }, { "tag", query } }, root);
    // https://api.douban.com/v2/book/search?q=<query>&apikey=0f4b40e701d84e822de1c2b50588eb86
    QVariantMap variant = root.toVariant().toMap();
    return get_booklist(variant);
}

http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Client::Config::Ptr Client::config() {
    return config_;
}

