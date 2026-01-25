#include <clover2/backend/api/v1/ros.hpp>
#include <drogon/HttpAppFramework.h>

#include <clover2/backend/model/nodes.hpp>

#include <nlohmann/json.hpp>

namespace api::v1 {

void ros::nodes([[maybe_unused]] const HttpRequestPtr &req,
                std::function<void(const HttpResponsePtr &)> &&callback) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);

    auto nodes = clover2::backend::model::nodes{};
    resp->setBody(nodes.serialize());

    callback(resp);
}

}  // namespace api::v1
