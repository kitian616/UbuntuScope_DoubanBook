#include <query.h>
#include <localization.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/Department.h>

#include <iomanip>
#include <iostream>
#include <sstream>

namespace sc = unity::scopes;

using namespace std;


/**
 * Define the larger "current weather" layout.
 *
 * The icons are larger.
 */

/**
 * Define the layout for the forecast results.
 *
 * The icon size is small, and ask for the card layout
 * itself to be horizontal. I.e. the text will be placed
 * next to the image.
 */
const static string BOOKLIST_TEMPLATE =
        R"(
            {
                "schema-version": 1,
                "template": {
                    "category-layout": "grid",
                    "card-layout": "vertical",
                    "card-size": "medium"
                },
                "components": {
                    "title": "title",
                    "art" : {
                        "field": "art"
                    },
                    "subtitle": "author"
                }
            }
        )";


Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Client::Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}


void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Get the query string
        string query_string = query.query_string();

        // Create the root department
        sc::Department::SPtr root_depts = sc::Department::create("", query, "搜索方式");
        sc::Department::SPtr  search_by_name_department = sc::Department::create("search_by_name", query, "按书名搜索");
        sc::Department::SPtr search_by_tag_department = sc::Department::create("search_by_tag", query, "按标签搜索");
        // Register these as subdepartments of the root
        root_depts->set_subdepartments({search_by_name_department,
                                       search_by_tag_department});
        // Register departments on the reply
        reply->register_departments(root_depts);

        /// Populate books category
        // the Client is the helper class that provides the results
        // without mixing APIs and scopes code.
        // Add your code to retreive xml, json, or any other kind of result
        // in the client.
        Client::BookRes booklist;

        if(query_string.empty()){
            query_string = "Harry Potter";
        }
        if( query.department_id().empty() || query.department_id() == "search_by_name"){
            // otherwise, get the current weather for the search string
            booklist = client_.search_books_by_name(query_string);
        }else if(query.department_id() == "search_by_tag"){
            // otherwise, get the current weather for the search string
            booklist = client_.search_books_by_tag(query_string);
        }

        // Register a category for the books, with the title we just built
        auto booklist_cat = reply->register_category("books_result", "", "",
                                                     sc::CategoryRenderer(BOOKLIST_TEMPLATE));
        // register_category(arbitrary category id, header title, header icon, template)
        // In this case, since this is the only category used by our scope,
        // it doesn’t need to display a header title, we leave it as a blank string.

        for (const auto &book : booklist.booklist) {
                    sc::CategorisedResult res(booklist_cat);

                    res.set_uri("http://book.douban.com/subject/" + book.id +"/");
                    res.set_art(book.avatar_url.large_url);
                    res.set_title(book.name);
                    res["id"] = book.id;
                    res["rating"] = book.rating.average;
                    res["numRaters"] = book.rating.numRaters;

                    std::string authos_string = "";
                    if(!book.authorlist.empty()) {
                        authos_string += book.authorlist.at(0).name;
                    }
                    for(unsigned int i = 1; i < book.authorlist.size(); ++i) {
                        authos_string += (", " + book.authorlist.at(i).name);
                    }
                    res["author"] = authos_string;
                    res["publisher"] = book.publisher;
                    res["pubdate"] = book.pubdate;

                    std::string tags_string = "";
                    if(!book.taglist.empty()) {
                        tags_string += book.taglist.at(0).title;
                    }
                    for(unsigned int i = 1; i < book.taglist.size(); ++i) {
                        tags_string += (", " + book.taglist.at(i).title);
                    }
                    tags_string = "标签：\r\n" + tags_string;
                    res["tags"] = tags_string;
                    res["catalog"] = book.catalog;
                    res["catalog"] = book.catalog;
                    // Push the result
                    if (!reply->push(res)) {
                        // If we fail to push, it means the query has been cancelled.
                        // So don't continue;
                        return;
                    }
                }
            } catch (domain_error &e) {
                // Handle exceptions being thrown by the client API
                cerr << e.what() << endl;
                reply->error(current_exception());
            }
        }


