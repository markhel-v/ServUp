

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <chrono>
#include <map>
#include <thread>
 

 
#include <string_view>
#include <nlohmann/json.hpp>


using namespace std;

using json = nlohmann::json;
const string COMMAND = "command";
const string USER_ID = "user_id";
const string MESSAGE = "message";
const string PRIVATE_MSG = "private_msg";
const string SET_NAME = "set_name";
const string NAME = "name";
 
//json j = R"( { "command": "set_name" ,"name" : gfgf })";
 
json j;
string a;
string userName;
 
// std::vector <uint8_t> v = json::to_msgpack(j);

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;
typedef client::connection_ptr connection_ptr;



class perftest {
public:
    typedef perftest type;
    typedef std::chrono::duration<int, std::micro> dur_type;

    perftest() {
        m_endpoint.set_access_channels(websocketpp::log::alevel::none);
        m_endpoint.set_error_channels(websocketpp::log::elevel::none);

        // Initialize ASIO
        m_endpoint.init_asio();

        // Register our handlers
       
        m_endpoint.set_message_handler(bind(&type::on_message, this, ::_1, ::_2));
        m_endpoint.set_open_handler(bind(&type::on_open, this, ::_1));
         
        m_endpoint.set_fail_handler(bind(&type::on_fail, this, ::_1));
    }

    void start(std::string uri) {
        websocketpp::lib::error_code ec;
         con = m_endpoint.get_connection(uri, ec);

        if (ec) {
            m_endpoint.get_alog().write(websocketpp::log::alevel::app, ec.message());
            return;
        }

        //con->set_proxy("http://humupdates.uchicago.edu:8443");

        m_endpoint.connect(con);

        // Start the ASIO io_service run loop
        
        m_endpoint.run();
    }
 

    context_ptr on_tls_init(websocketpp::connection_hdl) {
         
        context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv1);

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::single_dh_use);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return ctx;
    }

    void on_fail(websocketpp::connection_hdl hdl) {
        client::connection_ptr con = m_endpoint.get_con_from_hdl(hdl);

        std::cout << "Fail handler" << std::endl;
        std::cout << con->get_state() << std::endl;
        std::cout << con->get_local_close_code() << std::endl;
        std::cout << con->get_local_close_reason() << std::endl;
        std::cout << con->get_remote_close_code() << std::endl;
        std::cout << con->get_remote_close_reason() << std::endl;
        std::cout << con->get_ec() << " - " << con->get_ec().message() << std::endl;
    }

    void on_open(websocketpp::connection_hdl hdl) {
        cout << "connected." << endl;
      m_endpoint.send(hdl,  a, websocketpp::frame::opcode::text);
    }
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
        cout << " >> " + msg-> get_payload() << endl;
       //  m_endpoint.close(hdl, websocketpp::close::status::going_away, "");
    }
 

    void stop() {
        m_endpoint.stop();
    
    }

    void sendMessage(string message) {
        m_endpoint.send(con->get_handle(), message, websocketpp::frame::opcode::text);
    
    }
private:
    client m_endpoint;

    client::connection_ptr con;

 
};

int main(int argc, char* argv[]) {
    perftest endpoint;
    
    cout << " enter your name : ";
    getline(cin, userName);
    j[COMMAND] = SET_NAME;
    j[NAME] = userName;
    a = j.dump();
    auto thr = new thread([&]() {
        //std::string uri = "wss://echo.websocket.org";
        std::string uri = "ws://7afc8daa5694.ngrok.io";

        try {
            
            endpoint.start(uri);
        }
        catch (websocketpp::exception const& e) {
            std::cout << e.what() << std::endl;
        }
        catch (std::exception const& e) {
            std::cout << e.what() << std::endl;
        }
        catch (...) {
            std::cout << "other exception" << std::endl;
        }
        
        
        });
    string input;
    while(input.compare("exit") != 0) {
        getline(cin, input);
        endpoint.sendMessage(input);
    
    }
    endpoint.stop();
    thr->join();
   
}