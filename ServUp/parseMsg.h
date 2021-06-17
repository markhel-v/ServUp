#include <iostream>
#include <regex>
#include <algorithm>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include <string>
#include <map>


using json = nlohmann::json;


struct PerSocketData {
	unsigned long user_id;
	string name;
};


guarded_map<int, PerSocketData*>activeUsers;


typedef uWS::WebSocket<false, true, PerSocketData> UWEBSOCK;

const string COMMAND = "command";
const string USER_ID = "user_id";
const string MESSAGE = "message";
const string USER_FROM = "user_from";
const string PRIVATE_MSG = "private_msg";
const string ALL_MSG = "all_msg";
const string ADMIN = "admin";
const string SET_NAME = "set_name";
const string NAME = "name";
const string STATUS = "status";
const string ONLINE = "online";
const string BROADCAST = "broadcast";
const string TRAIN = "train";
const string NOTIFICATION = "notification";
const string SERVER = "server";
const string BOT = "bot";
const int  id_BOT = 1;
const int  id_SERV = 0;
 


string status(PerSocketData* data, bool b) {

	json request;
	request[COMMAND] = STATUS;
	request[NAME] = data->name;
	request[USER_ID] = data->user_id;
	request[ONLINE] = b;
	return request.dump();
}

bool isPrivateMessage(json parsed) {
	
	 return parsed[COMMAND] == PRIVATE_MSG;
}


bool isSetName(json parsed) {
	 
	return parsed[COMMAND] == SET_NAME;
}

void SetName(UWEBSOCK* ws,json parsed, PerSocketData* data) {
	json response;
	string user_name = parsed[NAME];
	size_t pos = user_name.find("$");
	if (pos == string::npos && user_name.length() <= 255) {
		data->name = user_name;
		cout << "set name new his name \n";
		activeUsers.set(data->user_id, data);

		ws->publish(BROADCAST, status(data, true));
	 
	}
	else {
		cout << "This name is not allowed!" << endl;
		response[COMMAND] = NOTIFICATION;
		response[USER_FROM] = id_SERV;
		response[NAME] = SERVER;
		response[MESSAGE] = "Incorrect name!";
        ws->send(response.dump(), uWS::OpCode::TEXT, true);
}
	 

}
auto createPrivateMessage(int id, string msg, string name) {
	json response;
	response[COMMAND] = PRIVATE_MSG;
	response[USER_FROM] = id;
	response[NAME] = name;
	response[MESSAGE] = msg;
	return response;
}


int parsePrivateId(json msg) {

	auto it = msg.find(USER_ID);

	if (it != msg.end())
	{
		cout << "Field USER_ID found!" << endl;
		// ПРИМЕЧАНИЕ: API гарантирует, что Key{JS} является строкой (в противном случае 
		// произойдет сбой). 

		return  it->get <int>();
	}
     
	else  cout << "Field USER_ID not found!" << endl;
}

 

string parsePrivateMessage(json msg) {              
	if (msg.find(MESSAGE) != msg.end()) {
		cout << "Field MESSAGE found!" << endl;

		return  msg.at(MESSAGE);
	}
	else  cout << "Field MESSAGE not found!" << endl;
}




void chatBotMessage(UWEBSOCK* ws, json parsed) {
	json resBot;
	resBot[COMMAND] = PRIVATE_MSG;
	resBot[USER_FROM] = id_BOT;
	resBot[NAME] = BOT;
	resBot[MESSAGE] = chat_bot(parsePrivateMessage(parsed));
	ws->send(resBot.dump(), uWS::OpCode::TEXT, true);
}






void  commandRouter(UWEBSOCK* ws, std::string_view message, uWS::OpCode opCode) {
	PerSocketData* data = ws->getUserData();
	json parsed = json::parse(message);
    
	   if (parsed.at(COMMAND) == PRIVATE_MSG) {
			int id = parsePrivateId(parsed);
			string user_msg = parsePrivateMessage(parsed);
			json response = createPrivateMessage(data->user_id, user_msg, data->name);
			ws->publish(USER_ID + to_string(id), response.dump());
		}
	   else if (parsed.at(COMMAND) == SET_NAME) {
			SetName(ws, parsed, data);
		}

	   else  cout << " NOT COMMAND...";
}







