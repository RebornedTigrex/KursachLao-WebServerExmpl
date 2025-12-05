#include "LambdaSenders.h"
#include "RequestHandler.h"
#include "ModuleRegistry.h"
#include "FileCache.h"
#include "macros.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
namespace po = boost::program_options;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
namespace fs = std::filesystem;

void printConnectionInfo(tcp::socket& socket) {
    try {
        tcp::endpoint remote_ep = socket.remote_endpoint();
        boost::asio::ip::address client_address = remote_ep.address();
        unsigned short client_port = remote_ep.port();

        std::cout << "Client connected from: "
            << client_address.to_string()
            << ":" << client_port << std::endl;
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "Error getting connection info: " << e.what() << std::endl;
    }
}

void CreateNewHandlers(RequestHandler* module, std::string staticFolder) {
    module->addRouteHandler("/test", [](const sRequest& req, sResponce& res) {
        if (req.method() != http::verb::get) {
            res.result(http::status::method_not_allowed);
            res.set(http::field::content_type, "text/plain");
            res.body() = "Method Not Allowed. Use GET.";
            return;
        }
        res.set(http::field::content_type, "text/plain");
        res.body() = "RequestHandler Module Scaling Test.\nAlso checking support for the Russian language.";
        res.result(http::status::ok);
        });

    module->addRouteHandler("/*", [](const sRequest& req, sResponce& res) {});
}

int main(int argc, char* argv[]) {
    // Объявление переменных для параметров
    std::string address;
    int port;
    std::string directory;

    // Настройка парсера аргументов
    po::options_description desc("Available options");
    desc.add_options()
        ("help,h", "Show help")
        ("address,a", po::value<std::string>(&address)->default_value("0.0.0.0"),
            "IP address to listen on")
        ("port,p", po::value<int>(&port)->default_value(8080),
            "Port to listen on")
        ("directory,d", po::value<std::string>(&directory)->default_value("static"),
            "Path to static files")
        ;

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (port <= 0 || port > 65535) {
            std::cerr << "Error: port must be in the range 1-65535\n";
            return EXIT_FAILURE;
        }

        // Проверка существования директории
        if (!fs::exists(directory)) {
            std::cerr << "Warning: directory '" << directory << "' does not exist\n";
            // Можно создать директорию:
            // fs::create_directories(directory);
        }
    }
    catch (const po::error& e) {
        std::cerr << "Argument parsing error: " << e.what() << "\n";
        std::cerr << desc << "\n";
        return EXIT_FAILURE;
    }

    // Вывод конфигурации
    std::cout << "Server configuration:\n"
        << " Address: " << address << "\n"
        << " Port: " << port << "\n"
        << " Directory: " << directory << "\n\n";

    // Остальная часть вашего кода...
    ModuleRegistry registry;
    auto* cacheModule = registry.registerModule<FileCache>(directory.c_str(), true, 100);
    auto* requestModule = registry.registerModule<RequestHandler>();
    CreateNewHandlers(requestModule, directory);
    registry.initializeAll();

    static_cast<RequestHandler*>(requestModule)->setFileCache(cacheModule);

    try {
        bool close = false;
        beast::error_code ec;
        beast::flat_buffer buffer;
        auto const net_address = net::ip::make_address(address);
        auto const net_port = static_cast<unsigned short>(port);
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor{ ioc, {net_address, net_port} };
        std::cout << "Server started on http://" << address << ":" << port << std::endl;

        for (;;) {
            auto socket = boost::make_shared<tcp::socket>(ioc);
            LambdaSenders::send_lambda<tcp::socket> lambda{ *socket.get(), close, ec };
            acceptor.accept(*socket);
            printConnectionInfo(*socket); //FIXME: Для получения 1 страницы он делает 6 запросов. Какого хрена? 
            http::request<http::string_body> req;
            http::read(*socket.get(), buffer, req, ec);
            if (ec == http::error::end_of_stream) continue;
            if (ec) break;
            requestModule->handleRequest(std::move(req), lambda);
            if (ec) break;
            if (close) continue;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}