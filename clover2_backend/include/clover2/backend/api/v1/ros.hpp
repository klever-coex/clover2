#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace api::v1 {

class ros : public drogon::HttpController<ros> {
public:
    METHOD_LIST_BEGIN
    METHOD_ADD(ros::nodes, "/nodes", Get);
    METHOD_LIST_END

    void nodes(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
};

}  // namespace api::v1
