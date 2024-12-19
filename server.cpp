#include <iostream>
#include <string>
#include <sstream>
#include <zmq.hpp>
#include <libpq-fe.h>
#include <vector>
#include <spdlog/spdlog.h> 
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>

const std::string SENTINEL = "~@~";

std::string connInfo;
PGconn* conn = nullptr;

std::vector<std::string> Split(std::string& str, const std::string& delimiter) 
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        tokens.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    tokens.push_back(str);
    return tokens;
}

void FormingReply(PGresult* res, std::string& reply)
{
    uint numOfRows = PQntuples(res);
    uint numOfCols = PQnfields(res);
    for (size_t row = 0; row < numOfRows; ++row) 
    {
        for (size_t col = 0; col < numOfCols; ++col)
        {
            char* column = PQgetvalue(res, row, col);
            reply += (column ? std::string(column) : "") + SENTINEL;
        }
    }
    PQclear(res);
}

void CheckTuplesResult(PGresult* pgr, PGconn* conn, std::string& replyStr,
    const std::shared_ptr<spdlog::logger>& logger)
{
    if (PQresultStatus(pgr) != PGRES_TUPLES_OK) 
    {
        std::cerr << "Database query failed: " << PQerrorMessage(conn) << std::endl;
        logger->error("Ошибка выполнения запроса: {}", PQerrorMessage(conn));
        logger->flush();
        PQclear(pgr);
        replyStr = PQerrorMessage(conn);
    } 
    else 
    {
        FormingReply(pgr, replyStr);
        logger->info("Запрос выполнен успешно.");
        logger->flush();
    }
}

void CheckCommandResult(PGresult* pgr, PGconn* conn, std::string& replyStr,
    const std::shared_ptr<spdlog::logger>& logger)
{
    if (PQresultStatus(pgr) != PGRES_COMMAND_OK) 
    {
        std::cerr << "Database query failed: " << PQerrorMessage(conn) << std::endl;
        logger->error("Ошибка выполнения команды: {}", PQerrorMessage(conn));
        logger->flush();
        PQclear(pgr);
        replyStr = PQerrorMessage(conn);
    } 
    else 
    {
        FormingReply(pgr, replyStr);
        logger->info("Команда выполнена успешно.");
        logger->flush();
    }
}

int main() 
{
    std::shared_ptr<spdlog::logger> logger =
        spdlog::create<spdlog::sinks::basic_file_sink_mt>("guitar_shop_log", "../guitar_shop.log", false);
    logger->set_level(spdlog::level::debug);

    // Создание контекста ZeroMQ
    zmq::context_t context(1);

    // Создаем сокет типа REP (ответ), который привязан к адресу tcp://*:5555
    zmq::socket_t socket(context, zmq::socket_type::rep);
    try 
    {
        socket.bind("tcp://*:5555");
        logger->info("Сокет ZeroMQ успешно привязан к порту 5555.");
        logger->flush();
    } 
    catch (const zmq::error_t& e) 
    {
        logger->error("Ошибка при создании сокета ZeroMQ: {}", e.what());
        logger->flush();
        return 1;
    }

    // Попытка установить соединение с базой данных PostgreSQL
    connInfo = "host=localhost dbname=guitarshop user=read_only password=read_only";
    conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) 
    {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
        logger->error("Connection to database failed: {}", PQerrorMessage(conn));
        logger->flush();
        PQfinish(conn);  // Закрываем соединение
        return 1;
    }
    logger->info("Соединение с базой данных установлено. Пользователь read_only.");
    logger->flush();

    while (true) 
    {
        zmq::message_t request;
        socket.recv(request);

        std::stringstream ss{static_cast<char*>(request.data())};

        std::string requestStr = ss.str();

        std::string replyStr;
        if (requestStr == "GetCategoryPreviews") 
        {
            std::string request = "SELECT path_to_preview, category_id, category_name FROM guitar_categories";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос {}", request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr == "Logout")
        {
            // очистить ресурсы из под входа в бД
            connInfo = "host=localhost dbname=guitarshop user=read_only password=read_only";
            conn = PQconnectdb(connInfo.c_str());
            if (PQstatus(conn) != CONNECTION_OK) 
            {
                std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
                logger->error("Connection to database failed: {}", PQerrorMessage(conn));
                logger->flush();
                PQfinish(conn);  // Закрываем соединение
                return 1;
            }
            logger->info("Соединение с базой данных установлено. Пользователь read_only.");
            logger->flush();
        }
        else if (requestStr.find("Login") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string user;
            if (words[1] == "admin") 
            {
                connInfo = "host=localhost dbname=guitarshop user=admin password=admin";
                user = "admin";
            }
            else 
            {
                connInfo = "host=localhost dbname=guitarshop user=basic_user password=basic_user";
                user = "basic_user";
            }
            
            conn = PQconnectdb(connInfo.c_str());
            if (PQstatus(conn) != CONNECTION_OK) 
            {
                std::cerr << "Connection to database failed: " << PQerrorMessage(conn) << std::endl;
                logger->error("Connection to database failed: {}", PQerrorMessage(conn));
                logger->flush();
                PQfinish(conn);  // Закрываем соединение
                return 1;
            }
            logger->info("Соединение с базой данных установлено. Пользователь " + user + '.');
            logger->flush();
        }
        else if (requestStr == "GetMostPopularCategory")
        {
            std::string request = "SELECT category_name FROM popular_categories LIMIT 1";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr == "GetAverageOrdersTotalAmount")
        {
            std::string request = "SELECT average_order_totals();";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetGuitarsByName") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT path_to_preview, guitar_attr_id, name, price, guitar_id FROM guitars WHERE name LIKE ";
            request += "'%" + words[1] + "%'";  

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetGuitars") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            // std::string request = "SELECT g.path_to_preview, g.guitar_attr_id, g.name, g.price, gc.path_to_photo";
            // request = request + " " + "FROM guitars AS g";
            // request = request + " " + "JOIN guitar_categories AS gc ON g.category_id = gc.category_id";
            // request = request + " " + "WHERE g.category_id = " + words[1];

            std::string request = "SELECT * FROM get_guitars_by_category(" + words[1] + ')';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();

            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetCurrGuitar") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            // std::string request = "SELECT ga.description, ga.num_of_strings, ga.design, ga.scale, ga.frame, ga.neck,";
            // request = request + " " + "ga.fingerboard, ga.fingerboard_radius, ga.color, ga.nut_width, ga.pegs,";
            // request = request + " " + "ga.fingerboard_sensor, ga.bridge_sensor, ga.electronics, ga.strings,";
            // request = request + " " + "ga.case_included, ga.country_of_origin, ga.in_stock, ga.path_to_photo, g.guitar_id";
            // request = request + " " + "FROM guitars as g";
            // request = request + " " + "JOIN guitar_attributes AS ga ON g.guitar_attr_id = ga.guitar_attr_id";
            // request = request + " " + "WHERE g.guitar_attr_id = " + words[1];
            
            std::string request = "SELECT * FROM Guitar_Details_View WHERE guitar_attr_id = " + words[1];

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetGuitarInStock") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            // std::string request = "SELECT ga.in_stock FROM guitars AS g JOIN guitar_attributes AS ga ";
            // request += "ON g.guitar_attr_id = ga.guitar_attr_id WHERE g.guitar_id =" + words[1] + ';';

            std::string request = "SELECT get_guitar_in_stock(" + words[1] + ')';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("CheckUserReg") != std::string::npos)
        {
            // 0 - "CheckUserReg", 1 - login, 2 - phoneNumber, 3 - email
            std::vector<std::string> words = Split(requestStr,SENTINEL);
            
            std::string request = "SELECT SUM(CASE WHEN login = '" + words[1];
            request += "' THEN 1 ELSE 0 END), SUM(CASE WHEN phone_number = '" + words[2];
            request += "' THEN 1 ELSE 0 END), SUM(CASE WHEN email = '" + words[3];
            request += "' THEN 1 ELSE 0 END) FROM users;";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("AddUser") != std::string::npos)
        {
            // 0 - "AddUser", 1 - login, 2 - password, 3 - name, 4 - phonenumber, 5 - emal    
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "INSERT INTO users (type, login, password, name, phone_number, email) VALUES (";
            request += "'user', '" + words[1] + "', '" + words[2] + "', '" + words[3] + "', '" + words[4] + "', '" + words[5];
            request += "');";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("CheckUserLog") != std::string::npos)
        {
            // 0 - "CheckUserLog", 1 - login, 2 - password
            std::vector<std::string> words = Split(requestStr,SENTINEL);
            
            std::string request = "SELECT COUNT(*) FROM users WHERE login = '" + words[1];
            request += "' AND password = '" + words[2] + "';";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetUserInfo") != std::string::npos)
        {
            // 0 - "GetUserInfo", 1 - login
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT login, password, name, phone_number, email, user_id ";
            request += "FROM users WHERE login = '" + words[1] + "';";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetCartItemQuantity") != std::string::npos)
        {
            // 0 - "GetCartItemQuantity", 1 - guitar_id, 2 - user_id
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT quantity FROM cart_items WHERE guitar_id = " + words[1];
            request += " AND user_id = " + words[2] + ';';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("DeleteCartItem") != std::string::npos)
        {
            // 0 - "DeleteCartItem", 1 - guitar_id, 2 - user_id
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "DELETE FROM cart_items WHERE guitar_id = " + words[1];
            request += " AND user_id = " + words[2] + ';';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("UpdateCartItemQuantity") != std::string::npos)
        {
            // 0 - "UpdateCartItemQuantity", 1 - guitar_id, 2 - user_id, 3 - action
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "UPDATE cart_items SET quantity = ";
            if (words[3] == "minus") {
                request += "quantity - 1 ";
            }
            else if (words[3] == "plus") {
                request += "quantity + 1 ";
            }
            else {
                request += words[3];
            }
            request += "WHERE guitar_id = " + words[1] + " AND user_id = " + words[2] + ';';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("CreateCartItem") != std::string::npos)
        {
            // 0 - "CreateCartItem", 1 - guitar_id, 2 - user_id
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "INSERT INTO cart_items (guitar_id, quantity, user_id) ";
            request += "VALUES (" + words[1] + ',' + '1' + ',' + words[2] + ')';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("AddGuitarAttributes") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string caseInclude = words[16] == "Да" ? "TRUE" : "FALSE"; 

            std::string request = "INSERT INTO guitar_attributes (description, num_of_strings, design, ";
            request += "scale, frame, neck, fingerboard, fingerboard_radius, color, nut_width, pegs, ";
            request += "fingerboard_sensor, bridge_sensor, electronics, strings, case_included, ";
            request += "country_of_origin, in_stock, path_to_photo) VALUES ('" + words[1] + "','";
            request += words[2] + "','" + words[3] + "','" + words[4] + "','" + words[5] + "','" + words[6] + "','";
            request += words[7] + "','" + words[8] + "','" + words[9] + "','" + words[10] + "','" + words[11] + "','";
            request += words[12] + "','" + words[13] + "','" + words[14] + "','" + words[15] + "','" + caseInclude;
            request += "','" + words[17] + "','" + words[18] + "','" + words[19] + "');";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetGuitarAttrId") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT guitar_attr_id FROM guitar_attributes WHERE ";
            request += "path_to_photo = '" + words[1] + "';";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("AddGuitar") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "INSERT INTO guitars (category_id, article, name, price, ";
            request += "path_to_preview, guitar_attr_id) VALUES ('" + words[1] + "','" + words[2];
            request += "','" + words[3] + "','" + words[4] + "','" + words[5] + "','" + words[6] + "');";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetCartItems") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT guitar_id, quantity FROM cart_items WHERE";
            request += " user_id = " + words[1] + ';';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetGuitarInfo") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT path_to_preview, guitar_attr_id, name, price";
            request = request + " " + "FROM guitars WHERE guitar_id = " + words[1];
            
            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("CreateOrder") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "INSERT INTO orders (user_id, order_date, shipping_address, status) ";
            request += "VALUES (" + words[1] + ',' + "NOW()" + ",'" + words[2] + "'," + "'created') ";
            request += "RETURNING order_id;";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("AddItemToOrder") != std::string::npos)
        {
            // 1 - order_id, 2 - user_id, 3 - guitar_id, 4-quantity
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "INSERT INTO order_items (order_id,user_id,guitar_id,quantity) ";
            request += "VALUES (" + words[1] + ',' + words[2] + ',' + words[3] + ',' + words[4] + ')';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("UpdateGuitarInStock") != std::string::npos)
        {
            // 1 - guitar_id, 2 - quantity
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "UPDATE guitar_attributes AS ga SET in_stock = " + words[2];
            request += " FROM guitars AS g WHERE g.guitar_attr_id = ga.guitar_attr_id";
            request += " AND g.guitar_id = " + words[1] + ";";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetGuitarCategory") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT gc.category_id FROM guitars AS g JOIN guitar_categories AS gc ";
            request += "ON g.category_id = gc.category_id WHERE g.guitar_id = " + words[1] + ';';

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("DeleteGuitar") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "DELETE FROM guitars WHERE guitar_id = " + words[1];

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckCommandResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetOrders") != std::string::npos)
        {
            std::string request = "SELECT * FROM orders";

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else if (requestStr.find("GetCurrOrderItems") != std::string::npos)
        {
            std::vector<std::string> words = Split(requestStr,SENTINEL);

            std::string request = "SELECT order_item_id, guitar_id, quantity FROM order_items ";
            request += "WHERE order_id = " + words[1];

            PGresult* res = PQexec(conn, request.c_str());
            logger->debug("Выполняется запрос " + request);
            logger->flush();
            CheckTuplesResult(res,conn,replyStr,logger);
        }
        else {
            replyStr = "Invalid request";
        }
        zmq::message_t reply(replyStr.data(), replyStr.size());
        socket.send(reply, zmq::send_flags::none);
    }

    PQfinish(conn);
    return 0;
}
