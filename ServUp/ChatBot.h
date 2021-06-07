#include <iostream>
#include <regex>
#include <algorithm>
#include <map>
#include <string>
#include <fstream>


using namespace std;

using json = nlohmann::json;
map <string, string> dbase{};



string toLower(string  text)
{
	transform(text.begin(), text.end(), text.begin(), ::tolower);
	return text;
}


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


string chat_bot(const string& question) {

	string query = toLower(question);
	string answer;
	int phraseCount = loadPhrases();
	bool found = false;
	for (const auto& entry : dbase) {
		regex pattern(".*" + entry.first + ".*");
		if (regex_match(query, pattern)) {
			found = true;
			answer = entry.second;
		}
	}
	if (!found || phraseCount == 0) answer = "Sorry, I can't answer that.";
	 
	return answer;
}

string response(const string& question) {

	for (auto entry : dbase) {
		regex pattern(".*" + entry.first + ".*");
		if (regex_match(toLower(question), pattern)) {
			return entry.second;
		}
	}
}

