#ifndef CFG_H
#define CFG_H
//#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <set>
#include <string> // description()

typedef std::vector<int32_t> cfgrule;
// the first element of cfgrule is the variable on the left
// the other elements of cfgrule is the string on the right

class cfg {
private:
	std::set<int32_t> v;
	std::set<int32_t> sigma; // terminal
	std::set<cfgrule> r;
	int32_t s;
	
	int32_t get_new_v(); // get new variable
	int32_t __get_new_v_data; // called by get_new_v()
public:
	cfg();
	~cfg();
	int add_v(int32_t x);
	int remove_v(int32_t x);
	int add_sigma(int32_t x);
	int remove_sigma(int32_t x);
	int add_r(int32_t left, const std::vector<int32_t> & right);
	int add_r(int32_t left, const char * right);
	int remove_r(int32_t left, const std::vector<int32_t> & right);
	int remove_r(int32_t left, const char * right);
	void set_s(int32_t x);

	void to_CNF();

	std::string description(int displayascii = 0); // used for debugging
	// set variable to nonzero to display ascii

};



#endif
