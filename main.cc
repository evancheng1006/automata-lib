#include <iostream>
#include "cfg.h"
int main() {
	cfg G;
/*
	G.add_v('S');
	G.add_sigma('0');
	G.add_sigma('1');
	G.add_r('S', "0S1");
	G.add_r('S', "");
*/
/*
	G.add_v('S');
	G.add_v('A');
	G.add_v('B');
	G.add_v('C');
	G.add_sigma('a');
	G.add_sigma('b');
	G.add_sigma('c');
	G.add_r('S', "AbB");
	G.add_r('S', "C");
	G.add_r('B', "AA");
	G.add_r('B', "AC");
	G.add_r('C', "b");
	G.add_r('C', "c");
	G.add_r('A', "a");
	G.add_r('A', "");
*/

	G.add_v('S');
	G.add_v('A');
	G.add_v('B');
	G.add_v('C');
	G.add_sigma('a');
	G.add_sigma('b');
	G.add_sigma('c');
	G.add_r('S', "ABC");
	G.add_r('A', "B");
	G.add_r('B', "C");
	G.add_r('C', "A");
	G.add_r('C', "BB");
	G.add_r('A', "a");
	G.add_r('B', "b");
	G.add_r('C', "c");


	std::cout << G.description(1) << "\n";
	G.to_CNF();
	std::cout << G.description(1) << "\n";
	return 0;
}
