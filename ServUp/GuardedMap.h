     
#include <iostream>
#include <uwebsockets/App.h>
#include <thread>
#include <map>
#include <algorithm>
using namespace std;
 
template <class K, class V, class Compare = std::less<K>, class Allocator = std::allocator<std::pair<const K, V> > >
class guarded_map {
private:
	std::map<K, V, Compare, Allocator> _map;
	std::mutex _m;

public:
	void set(K key, V value) {
		std::lock_guard<std::mutex> lk(this->_m);
		this->_map[key] = value;
	}

	V& get(K key) {
		std::lock_guard<std::mutex> lk(this->_m);
		return this->_map[key];
	}
	
	
	void erase(K key){
		std::lock_guard<std::mutex> lk(this->_m);
		this->_map.erase(key);
		
		
	}

	bool empty() {
		std::lock_guard<std::mutex> lk(this->_m);
		return this->_map.empty();
	}

	vector<string> getNames() {
		std::lock_guard<std::mutex> lk(this->_m);
		vector<string> result;
		for (auto entry : this->_map) {
			result.push_back("Active User " + entry.second->name + ", id: " + to_string(entry.first));
		}
		return result;
	}

	// other public methods you need to implement
};
 