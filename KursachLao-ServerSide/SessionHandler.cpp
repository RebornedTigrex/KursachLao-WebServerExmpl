#include "SessionHandler.h"
#include "RequestHandler.h"
#include <iostream>

// Реализация метода отправки сообщений
template<class Stream>
template<bool isRequest, class Body, class Fields>
void SessionHandler::send_lambda<Stream>::operator()(
    http::message<isRequest, Body, Fields>&& msg) const
{
    close_ = msg.need_eof();
    http::serializer<isRequest, Body, Fields> sr{ msg };
    http::write(stream_, sr, ec_);
}

// Реализация обработчика запроса - делегируем RequestHandler
template<class Body, class Allocator, class Send>
void SessionHandler::handle_request(
    http::request<Body, http::basic_fields<Allocator>>&& req,
    Send&& send)
{
    // Получаем экземпляр RequestHandler и делегируем обработку
    auto* request_handler = RequestHandler::getInstance();
    if (request_handler) {
        request_handler->handleRequest(std::move(req), std::forward<Send>(send));
    }
    else {
        // Fallback: если RequestHandler не доступен
        http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req.keep_alive());
        res.body() = "RequestHandler not available";
        res.prepare_payload();
        return send(std::move(res));
    }
}

// Явные инстанциования шаблонных методов
template void SessionHandler::handle_request<
    http::string_body,
    std::allocator<char>,
    SessionHandler::send_lambda<tcp::socket>>(
        http::request<http::string_body, http::basic_fields<std::allocator<char>>>&&,
        SessionHandler::send_lambda<tcp::socket>&&);

template class SessionHandler::send_lambda<tcp::socket>;

// Асинхронная реализация обработки сессии
void SessionHandler::do_session(tcp::socket socket)
{
    auto self = std::make_shared<session_data>(std::move(socket));

    // Запускаем асинхронное чтение
    self->read_request();
}

// Вспомогательная структура для хранения состояния сессии
struct session_data {
    tcp::socket socket;
    beast::flat_buffer buffer{ 8192 };
    http::request<http::string_body> req;
    std::shared_ptr<void> res;
    bool close = false;
    beast::error_code ec;

    explicit session_data(tcp::socket&& socket)
        : socket(std::move(socket)) {
    }

    void read_request() {
        // Очищаем запрос для повторного использования
        req = {};

        // Асинхронное чтение запроса
        http::async_read(socket, buffer, req,
            [self = shared_from_this()](beast::error_code ec, std::size_t bytes_transferred) {
                self->on_read(ec, bytes_transferred);
            });
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        if (ec == http::error::end_of_stream) {
            // Клиент закрыл соединение
            return do_close();
        }

        if (ec) {
            // Ошибка чтения
            return;
        }

        // Обрабатываем запрос
        handle_request(std::move(req),
            SessionHandler::send_lambda<tcp::socket>{socket, close, ec});

        if (ec) {
            return;
        }
        if (close) {
            return do_close();
        }

        // Читаем следующий запрос
        read_request();
    }

    void do_close() {
        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);
    }
};