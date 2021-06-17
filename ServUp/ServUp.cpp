
#include <nlohmann/json.hpp>
#include <uwebsockets/App.h>
#include "ChatBot.h"
#include <thread>
#include <format>
#include "guardedMap.h"
#include "parseMsg.h" 

 using namespace std ;    


 

int main()
{
	PerSocketData bot { 1, BOT };
	PerSocketData server { 0, SERVER };



	atomic_ulong  latest_id = 10 ;
	atomic_int  n_clients{ 0 };

	vector<thread*> threads{ thread::hardware_concurrency() };

	transform(threads.begin(), threads.end(), threads.begin(), [&](auto* thr) {
		return new thread([&latest_id, &n_clients]() {

			uWS::App().ws<PerSocketData>("/*", {          // /* ws->getUserData returns one of these */
					/* Settings */
					.compression = uWS::SHARED_COMPRESSOR,
					.maxPayloadLength = 16 * 1024 * 1024,
					.idleTimeout = 1024,
					.maxBackpressure = 1 * 1024 * 1024,
					.closeOnBackpressureLimit = false,
					.resetIdleTimeoutOnSend = false,
					.sendPingsAutomatically = true,

					/* Handlers */

						.upgrade = nullptr,
						.open = [&](auto* ws) {
						   PerSocketData* data = ws->getUserData();
						   data->user_id = latest_id++;
		                 cout<<format("User id:{} connected\nTotal users online:{}\n",data->user_id,(++n_clients));
			 
						 // Bcем сообщаем что он подкл
						 ws->publish(BROADCAST, status(data, true));
							   ws->subscribe(BROADCAST);
							   ws->subscribe(USER_ID + to_string(data->user_id));
							   
							   // cообщаем ноBому юзеру о уже подключенных юзерах  
								 for (auto entry : activeUsers.getNames()) {
								  ws->send(entry, uWS::OpCode::TEXT);
								 }

								 activeUsers.set(data->user_id, data);       //add user in map  

									 },
						  .message = [&](auto* ws,  std::string_view message, uWS::OpCode opCode) {
							   PerSocketData* data = ws->getUserData();
						       cout << format("Message from id {}{} \n", data->user_id, message);
							   
							   json parsed = json::parse(message);
							   if (parsed.find(USER_ID) != parsed.end()) {
								   if (parsed.at(USER_ID) == id_BOT)
									   chatBotMessage(ws, parsed);
							   }
							   
							   if (parsed.find(COMMAND) != parsed.end())
							   commandRouter(ws, message, uWS::OpCode::TEXT);

									 },

						  .close = [ &](auto* ws, int /*code*/, std::string_view /*message*/) {
						   PerSocketData* data = ws->getUserData();
							  cout << format("closed User id:{}\n",data->user_id);
							  ws->publish(BROADCAST, status(data, false));  //   offline status msg all
							  activeUsers.erase(data->user_id); // delete users from map
							 
							  --n_clients;
									 }
				}).listen(9001, [](auto* listen_socket) {
										 if (listen_socket) {
											 greeting();
											 cout << "Thread " << std::this_thread::get_id() << " listening on port " << 9001 << endl;
											 cout << "Chat-bot entered the chat! (id: 1) " << endl;

										 }
										 else { cout << "Server failed to start :( " << endl; }

					}).run();

			});





		});


	for_each(threads.begin(), threads.end(), [](auto* thr) {
		thr->join();
		});

}








