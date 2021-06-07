 
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include "ChatBot.h"


using namespace std;


const string COMMAND = "command";
const string USER_ID = "user_id";
const string MESSAGE = "message";
const string USER_FROM = "user_from";
const string PRIVATE_MSG = "private_msg";
const string SET_NAME = "set_name";
const string NAME = "name";
const string STATUS = "status";
const string ONLINE = "online";
const string BROADCAST = "broadcast";
const string TRAIN = "train";
const string NOTIFICATION = "notification";
const string SERVER = "server";
const string BOT = "bot";


struct PerSocketData {
	unsigned long user_id;
	string name;
};

map<int, PerSocketData*>activeUsers;


typedef uWS::WebSocket<false, true, PerSocketData> UWEBSOCK;


string status(PerSocketData* data, bool online) {
	json request;
	request[COMMAND] = STATUS;
	request[NAME] = data->name;
	request[USER_ID] = data->user_id;
	request[ONLINE] = true;
	return request.dump();
}

void processMessage(UWEBSOCK* ws, std::string_view message, int latest_id, map<int, PerSocketData*>& users, uWS::OpCode opCode) {
	PerSocketData* data = ws->getUserData();

	json parsed = json::parse(message);
    json response;
	string command = parsed[COMMAND];
	
	if (command == PRIVATE_MSG) {
		int user_id = parsed[USER_ID];
		string user_msg = parsed[MESSAGE];


		if (user_id == 1) {
			
			response[COMMAND] = PRIVATE_MSG;
			response[USER_FROM] = user_id;
			response[NAME] = BOT;
			response[MESSAGE] = chat_bot(user_msg);
			ws->send(response.dump(), opCode, true);
		}
		else if (users.find(user_id) != users.end()) {
			response[COMMAND] = PRIVATE_MSG;
			response[USER_FROM] = data->user_id;
			if (data->name != "NULL") response[NAME] = data->name;
			response[MESSAGE] = user_msg;
			ws->publish("user_id" + to_string(user_id), response.dump());
		}


		else {
			cout << "Error! There is no user with ID = " << user_id << "!" << endl;
			response[COMMAND] = NOTIFICATION;
			response[USER_FROM] = 0;
			response[NAME] = SERVER;
			response[MESSAGE] = "User not found!";
			ws->send(response.dump(), opCode, true);
		}
	}
	if (command == SET_NAME) {
		string user_name = parsed[NAME];
		int pos = user_name.find("::");
		if (pos == -1 && user_name.length() <= 255) {
			data->name = user_name;
			cout << "User № " << data->user_id << " set his name to " << data->name << endl;
		    ws->publish(BROADCAST, status(data, false));
		}
		else {
			cout << "This name is not allowed!" << endl;
			response[COMMAND] = NOTIFICATION;
			response[USER_FROM] = 0;
			response[NAME] = SERVER;
			response[MESSAGE] = "Incorrect name!";
			ws->send(response.dump(), opCode, true);
		}
	}
}
int main()
{
	/* ws->getUserData returns one of these */
	PerSocketData bot { 1, BOT };
	PerSocketData server { 0, SERVER };

	unsigned long latest_id = 10;
	unsigned n_clients = 0;
	uWS::App().ws<PerSocketData>("/*", {
			.idleTimeout = 9999,

			.open = [&](auto* ws) {
      		   PerSocketData* data = ws->getUserData();
			   data->user_id = latest_id++;
			   cout << "User id:"<< data->user_id << " connected\n";
			   cout << "Total users connected  " << ++n_clients << "\n";

				   ws->publish(BROADCAST, status(data, true)); // Bcем сообщаем что он подкл
				   ws->subscribe(BROADCAST);
				   ws->subscribe("user_id" + to_string(data->user_id));
				   for (auto entry : activeUsers) {
					   ws->send(status(entry.second, true), uWS::OpCode::TEXT);
				   }

				   activeUsers[data->user_id] = data; // адд B карту юзкра

					   },
			.message = [&](auto* ws,  std::string_view message, uWS::OpCode opCode) {
			PerSocketData* data = ws->getUserData();
			cout << "Message from N " << data->user_id << ": " << message << endl;
			processMessage(ws,message,latest_id, activeUsers, opCode);

					   },

			.close = [&](auto* ws, int /*code*/, std::string_view /*message*/) {
			 PerSocketData* data = ws->getUserData();
				cout << "closed User id: " <<data->user_id << "\n";
			 
			   ws->publish(BROADCAST, status(data, false));  // отключился
			   activeUsers.erase(data->user_id); // удаление их карты юзера
			   --n_clients;
					   }
		}).listen(9001, [](auto* listen_socket) {
						   if (listen_socket) {
							   std::cout << "Listening on port " << 9001 << std::endl;
							   cout << "Chat-bot entered the chat! (User № 1) " << endl;
							   greeting();
						   }
			}).run();

}








