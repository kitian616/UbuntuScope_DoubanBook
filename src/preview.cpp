#include <preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>
#include <sstream>

namespace sc = unity::scopes;

using namespace std;

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}

double Preview::get_rating_for_star(const double rating) {
    double rating_value = rating / 2;
    double rating_integer = (int)rating_value;
    double rating_decimal = rating_value - rating_integer;
    double rating_for_star= 0.0;
    if (rating_decimal <= 0.2) {
       rating_for_star =  rating_integer;
    }
    else if (rating_decimal > 0.2 && rating_decimal < 0.8){
        rating_for_star = rating_integer + 0.5;
    }
    else {
        rating_for_star = rating_integer + 1;
    }
    return rating_for_star;
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();

    // Support three different column layouts
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);

    // We define 3 different layouts, that will be used depending on the
    // device. The shell (view) will decide which layout fits best.
    // If, for instance, we are executing in a tablet probably the view will use
    // 2 or more columns.
    // Column layout definitions are optional.
    // However, we recommend that scopes define layouts for the best visual appearance.

    // Single column layout
    layout1col.add_column( { "image_widget", "header_widget", "stars_widget", "summary_widget", "actions_widget" } );

    // Two column layout
    layout2col.add_column( { "image_widget" } );
    layout2col.add_column( { "header_widget", "stars_widget", "summary_widget", "actions_widget" } );

    // Three cokumn layout
    layout3col.add_column( { "image_widget" });
    layout3col.add_column( { "header_widget",  "stars_widget""summary_widget",  } );
    layout3col.add_column( { "actions_widget" } );

    // Register the layouts we just created
    reply->register_layout( { layout1col, layout2col, layout3col } );

    sc::VariantBuilder builder;
    // Define the image section
    sc::PreviewWidget image("image_widget", "image");
    // It has a single "source" property, mapped to the result's "art" property
    image.add_attribute_mapping("source", "art");

    // Define the header section
    sc::PreviewWidget header("header_widget", "header");
    // It has a "title" and a "subtitle" property
    header.add_attribute_mapping("title", "title");
    header.add_attribute_value("subtitle", sc::Variant(result["author"].get_string() + "\r\n" + result["publisher"].get_string()));
    // Define the summary section
    sc::PreviewWidget summary("summary_widget", "text");
    // It has a "text" property, mapped to the result's "description" property
    summary.add_attribute_mapping("text", "tags");


    sc::PreviewWidget w_reviews("stars_widget", "reviews");
    std::ostringstream numRaters_sstream;
    numRaters_sstream << result["numRaters"].get_int() << "人评价";
    std::string numRaters_string = numRaters_sstream.str();
    builder.add_tuple({
        {"author", sc::Variant(result["rating"])},
        {"review",  sc::Variant(numRaters_string)},
        {"rating", sc::Variant(get_rating_for_star(result["rating"].get_double()))},
    });
    w_reviews.add_attribute_value("reviews", builder.end());

    // Define the actions section
    sc::PreviewWidget w_actions("actions_widget", "actions");
    //sc::VariantBuilder builder;
    builder.add_tuple({
        {"id", sc::Variant("open")},
        {"label", sc::Variant("Open")},
        {"uri", result["uri"]},
    });
    w_actions.add_attribute_value("actions", builder.end());

    // Push each of the sections
    reply->push( { image, header, summary, w_actions, w_reviews } );
}

