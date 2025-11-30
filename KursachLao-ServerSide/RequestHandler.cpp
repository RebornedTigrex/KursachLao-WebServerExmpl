#include "RequestHandler.h"
#include <iostream>

std::unique_ptr<RequestHandler> RequestHandler::instance_ = nullptr;

RequestHandler* RequestHandler::getInstance() {
    return instance_.get();
}

RequestHandler::RequestHandler()
    : BaseModule("HTTP Request Handler") {
}

bool RequestHandler::onInitialize() {
    setupDefaultRoutes();
    std::cout << "RequestHandler initialized with " << routeHandlers_.size() << " routes" << std::endl;
    return true;
}

void RequestHandler::onShutdown() {
    routeHandlers_.clear();
    std::cout << "RequestHandler shutdown" << std::endl;
}

void RequestHandler::addRouteHandler(const std::string& path,
    std::function<void(const http::request<http::string_body>&, http::response<http::string_body>&)> handler) {
    routeHandlers_[path] = handler;
}

void RequestHandler::setupDefaultRoutes() {
    // Обработчик для корневого пути
    addRouteHandler("/", [](const http::request<http::string_body>& req, http::response<http::string_body>& res) {
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/plain");
        res.body() = "Hello from RequestHandler module!";
        });

    // Обработчик для /status
    addRouteHandler("/status", [](const http::request<http::string_body>& req, http::response<http::string_body>& res) {
        res.result(http::status::ok);
        res.set(http::field::content_type, "application/json");
        res.body() = R"({"status": "ok", "service": "modular_http_server"})";
        });
}