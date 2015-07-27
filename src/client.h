#ifndef CLIENT_H_
#define CLIENT_H_

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

#include <QJsonDocument>
#include <QVariantMap>

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Client {
public:

    /**
     * Client configuration
     */
    struct Config {
        typedef std::shared_ptr<Config> Ptr;

        // The root of all API request URLs
        std::string apiroot { "https://api.douban.com/v2" };

        // The custom HTTP user agent string for this library
        std::string user_agent { "example-network-scope 0.1; (foo)" };
    };

    /** DATA STRUCTURE BEGIN **/
     struct  Author{
         std::string name;
     };
    typedef std::deque<Author> Authors;
    /**
     * Information about a Book
     */
     struct Book_avatar{
         std::string small_url;
         std::string large_url;
         std::string medium_url;
     };

    struct Book_tag{
        int count;
        std::string name;
        std::string title;
    };
    typedef std::deque<Book_tag> Tags;

    struct Book_rating {
        int numRaters;
        double average;
    };

    struct Book {
        std::string id;
        std::string name;               // book name
        Book_rating rating;
        Authors authorlist;             // book authors
        std::string publisher;               // book name
        Tags taglist;
        std::string pubdate;
        std::string original_name;  // original name of the book
        Book_avatar avatar_url;         //  cover of the book
        std::string url;                    //url of the book
        std::string catalog;            //catalog of the book
    };

    /**
     * A list of Book information
     */
    typedef std::deque<Book> Books;

    /**
     * Result of Book information
     */
    struct BookRes{
        Books booklist;
    };
    /** DATA STRUCTURE END **/
public:
    Client(Config::Ptr config);

    virtual ~Client() = default;

    /** USER FUNCTION BEGIN **/
    /**
     * Get the book info
     */
    virtual BookRes  search_books_by_name(const std::string &query);
    virtual BookRes  search_books_by_tag(const std::string &query);
private:
    BookRes get_booklist(const QVariantMap &variant);
   /** USER FUNCTION END **/

public:
    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual Config::Ptr config();

protected:
    void get(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             QJsonDocument &root);
    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    /**
     * Hang onto the configuration information
     */
    Config::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;
};

#endif // CLIENT_H_

