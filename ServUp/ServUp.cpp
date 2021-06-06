#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include <regex>
#include <map>
#include <algorithm>


using namespace std;
using json = nlohmann::json;


class ChatBot {

	string toLower(string  text) const
	{
		transform(text.begin(), text.end(), text.begin(), ::tolower);
		return text;
	}
	map <string, string> dbase{};

	int loadPhrases() {

		ifstream phrases("dbase.txt");
		string line;
		int len = 0;
		while (getline(phrases, line)) {
			string delimiter = " $ ";
			int pos = line.find(delimiter);
			string question = line.substr(0, pos);
			string answer = line.substr(pos + delimiter.length());
			dbase.insert(pair<string, string>(question, answer));
			len++;
		}
		return len;
	}
public:


	ChatBot() {
		int len = loadPhrases();
		cout << "chat_bot loaded " << len << " phrases\n";
	}

	string response(const string& question) const {

		for (auto entry : dbase) {
			regex pattern(".*" + entry.first + ".*");
			if (regex_match(toLower(question), pattern)) {
				return entry.second;
			}
		}
	}

};

const string COMMAND = "command";
const string USER_ID = "user_id";
const string MESSAGE = "message";
const string USER_FROM = "user_from";
const string PRIVATE_MSG = "private_msg";
const string SET_NAME = "set_name";
const string NAME = "name";
const string STATUS = "status";
const string ONLINE = "online";
const string BROADCAST = "broadcat";


struct PerSocketData {
	int user_id;
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







void processMessage(UWEBSOCK* ws, std::string_view message, int latest_id, const ChatBot& chat_bot) {
	PerSocketData* data = ws->getUserData();



	auto parsed = json::parse(message);
	string command = parsed[COMMAND];

	//клиент
	if (command == PRIVATE_MSG) {
		int user_id = parsed[USER_ID];

		if (user_id == 1)
		{

			string user_msg = parsed[MESSAGE];
			json response;
			response[COMMAND] = PRIVATE_MSG;
			response[USER_FROM] = data->user_id;
			response[MESSAGE] = chat_bot.response(parsed[MESSAGE]);

			ws->publish("user_id" + to_string(user_id), response.dump());
			return;

		}

		if (command == SET_NAME) {
			string user_name = parsed[NAME];

			if (user_name.size() > 255) // HW 3
				return;

			std::size_t found = user_name.find("::");
			if (found != std::string::npos) // HW 2
				return;

			data->name = parsed[NAME];
		}
		string user_msg = parsed[MESSAGE];
		json response; //   { "command": "private_msg", "user_from" : 14, "message" : "Привет, двенатсатй!" }
		response[COMMAND] = PRIVATE_MSG;
		response[USER_FROM] = data->user_id;
		response[MESSAGE] = user_msg;
		ws->publish("user_id" + to_string(user_id), response.dump());


	}
}
int main()
{
	/* ws->getUserData returns one of these */
	ChatBot chat_bot;

	int latest_id = 10;
	unsigned n_clients = 0;
	uWS::App().ws<PerSocketData>("/*", {
			.idleTimeout = 1024,

			.open = [&](auto* ws) {

			   PerSocketData* data = ws->getUserData();
				   data->user_id = latest_id++;
				   cout << "User N" << data->user_id << "connected\n ";
				   cout << "Total users connected" << ++n_clients << "\n";
				   ws->publish(BROADCAST, status(data, true)); // Bcем сообщаем что он подкл

				   ws->subscribe(BROADCAST);
				   ws->subscribe("user_id" + to_string(data->user_id));




				   for (auto entry : activeUsers) {
					   ws->send(status(entry.second, true),uWS::OpCode::TEXT);
				   }

				   activeUsers[data->user_id] = data; // адд B карту юзкра





					   },
			.message = [&](auto* ws,  std::string_view message, uWS::OpCode opCode) {
			PerSocketData* data = ws->getUserData();
			cout << "Message from N " << data->user_id << ": " << message << endl;
			 processMessage(ws,message,latest_id,chat_bot);



					   },

			.close = [&](auto* ws, int /*code*/, std::string_view /*message*/) {
			   cout << "close\n";
			   PerSocketData* data = ws->getUserData();
			   ws->publish(BROADCAST, status(data, false));  // отключился
			   activeUsers.erase(data->user_id); // удаление их карты юзера
			   --n_clients;
					   }
		}).listen(9001, [](auto* listen_socket) {
						   if (listen_socket) {
							   std::cout << "Listening on port " << 9001 << std::endl;
						   }
			}).run();

}









