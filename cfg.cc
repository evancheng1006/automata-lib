#include <map>
#include <iostream>
#include "cfg.h"
cfg::cfg() {
	s = (int32_t)'S';
	__get_new_v_data = (int32_t)'A';
	// 'space' is the first printable ASCII character
}
cfg::~cfg() {

}
int cfg::add_v(int32_t x) {
	if (sigma.find(x) != sigma.end()) {
		return -1; // error, x already found in sigma
	} else {
		v.insert(x);
	}
	return 0;
}
int cfg::add_sigma(int32_t x) {
	if (v.find(x) != v.end()) {
		return -1; // error, x already found in v
	} else {
		sigma.insert(x);
	}
	return 0;
}
int cfg::remove_v(int32_t x) {
	if (v.erase(x) == 0) {
		return -1; // error, x not found
	}
	return 0;
}
int cfg::remove_sigma(int32_t x) {
	if (sigma.erase(x) == 0) {
		return -1; // error, x not found
	}
	return 0;
}
int cfg::add_r(int32_t left, const std::vector<int32_t> & right) {
// will return -1 if left is not found in current variables. However, the insertion will still be done despite of the error.
	cfgrule tmp;
	tmp.push_back(left);
	// concat
	tmp.insert(tmp.end(), right.begin(), right.end());
	r.insert(tmp);
	if (v.find(left) == v.end()) {
		return -1; // error left not found
	}
	return 0;
}
int cfg::add_r(int32_t left, const char * right) {
	std::vector<int32_t> tmp;
	while (*right != '\0') {
		tmp.push_back(*right);
		right++;
	}
	return add_r(left, tmp);
}
int cfg::remove_r(int32_t left, const std::vector<int32_t> & right) {
// will return -1 if left is not found in current variables. However, the insertion will still be done despite of the error.
	cfgrule tmp;
	tmp.push_back(left);
	// concat
	tmp.insert(tmp.end(), right.begin(), right.end());
	if (r.erase(tmp) == 0) {
		return -1; // error, rule not found
	}
	return 0;
}
int cfg::remove_r(int32_t left, const char * right) {
	std::vector<int32_t> tmp;
	while (*right != '\0') {
		tmp.push_back(*right);
		right++;
	}
	return remove_r(left, tmp);
}
void cfg::set_s(int32_t x) {
	s = x;
	return;
}

int32_t cfg::get_new_v() {
// not a reentrant function, be careful, __get_new_v_data may be modified.
	// need a unique new variable
	do {
		__get_new_v_data++;
	} while (v.find(__get_new_v_data) != v.end() || sigma.find(__get_new_v_data) != sigma.end());
	v.insert(__get_new_v_data);
	return __get_new_v_data;
}

void cfg::to_CNF() {
// reference: en.wikipedia.org/wiki/Chomsky_normal_form
// ORDER of transformations: 
// When choosing the order in which the above transformations are to be applied, it has to be considered that some transformations may destroy the result achieved by other ones. For example, START will re-introduce a unit rule if it is applied after UNIT. The table shows which orderings are admitted.
// Moreover, the worst-case bloat in grammar size[note 5] depends on the transformation order. Using |G| to denote the size of the original grammar G, the size blow-up in the worst case may range from |G|2 to 22 |G|, depending on the transformation algorithm used.[6]:7 The blow-up in grammar size depends on the order between DEL and BIN. It may be exponential when DEL is done first, but is linear otherwise. UNIT can incur a quadratic blow-up in the size of the grammar.[6]:5 The orderings START,TERM,BIN,DEL,UNIT and START,BIN,DEL,UNIT,TERM lead to the least (i.e. quadratic) blow-up.
// START: Eliminate the start symbol from right-hand sides
	to_CNF_START();
// TERM: Eliminate rules with nonsolitary terminals
	to_CNF_TERM();
// BIN: Eliminate right-hand sides with more than 2 nonterminals
	to_CNF_BIN();
// DEL: Eliminate epsilon-rules
	to_CNF_DEL();
// UNIT: Eliminate unit rules
	to_CNF_UNIT();
}

void cfg::to_CNF_START() {
// START: Eliminate the start symbol from right-hand sides
	std::vector<int32_t> right;
	int32_t S_0 = get_new_v();
	right.clear();
	right.push_back(s);
	//add_v(S_0);
	add_r(S_0, right);
	s = S_0;
}

void cfg::to_CNF_TERM() {
// TERM: Eliminate rules with nonsolitary terminals

	std::map<int32_t, int32_t> term_to_new_var;
	std::map<int32_t, int32_t> new_var_to_term; 
	// new variables for all terminals
	// term_to_new_var use terminals to find new variables
	// new_var_to_term use new variables to find terminals
	std::set<cfgrule> TERMresult;
	for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
		cfgrule tmp_rule;
		tmp_rule = *it;
		for (unsigned int i = 1; i < tmp_rule.size(); i++) {
			int32_t tmpint = tmp_rule[i];
			if (sigma.find(tmpint) != sigma.end()) {
				// is a terminal
				if (term_to_new_var.find(tmpint) == term_to_new_var.end()) {
					// create new variable for this terminal
					int32_t tmpnewvar = get_new_v();
					//add_v(tmpnewvar); 
					term_to_new_var[tmpint] = tmpnewvar;
					new_var_to_term[tmpnewvar] = tmpint;
				}
				tmp_rule[i] = term_to_new_var[tmpint];
			}
		}
		TERMresult.insert(tmp_rule);
	}
	for (std::map<int32_t, int32_t>::iterator it = new_var_to_term.begin(); it != new_var_to_term.end(); it++) {
		cfgrule tmp_rule;
		tmp_rule.resize(2);
		tmp_rule[0] = it->first;
		tmp_rule[1] = it->second;
		TERMresult.insert(tmp_rule);
	}
	r = TERMresult;
	TERMresult.clear();
}

void cfg::to_CNF_BIN() {
// BIN: Eliminate right-hand sides with more than 2 nonterminals
	std::set<cfgrule> BINresult;
	for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
		cfgrule tmp_rule;
		tmp_rule = *it;
		if (tmp_rule.size() > 3) {
			// more than 2 nonterminals
			cfgrule tmp_new_rule;
			tmp_new_rule.resize(3);
			tmp_new_rule[0] = tmp_rule[0];

			for (unsigned int i = 2; i < tmp_rule.size(); i++) {
				tmp_new_rule[1] = tmp_rule[i - 1];
				if ((i + 1) < tmp_rule.size()) {
					tmp_new_rule[2] = get_new_v();
				} else {
					tmp_new_rule[2] = tmp_rule[i];
				}
				BINresult.insert(tmp_new_rule);
				tmp_new_rule[0] = tmp_new_rule[2];
				//add_r(new_left, new_right);
				//new_left = new_right[1];
			}
		} else {
			BINresult.insert(tmp_rule);
		}
	}
	r = BINresult;
	BINresult.clear();
}

void cfg::to_CNF_DEL() {
// DEL: Eliminate epsilon-rules
	std::set<cfgrule> DELresult;
	std::set<int32_t> nullable;
	// DEL_find_nullable
	int cont_loop = 1; // if in an iteration, we cannot find a new nullable character the stop looping
	// after START, we know that starting variable will not generate null string in one step only.
	while (cont_loop) {
		cont_loop = 0;
		// direct rule
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (it->size() == 1 && (*it)[0] != s) {
				if (nullable.find((*it)[0]) == nullable.end()) {
					// insert new nullable
					nullable.insert((*it)[0]);
					cont_loop = 1;
				}
			}
		}
		// indirect rule
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (nullable.find((*it)[0]) == nullable.end() && it->size() > 1 && (*it)[0] != s) {
				int is_nullable = 1;
				for (unsigned int i = 1; i < it->size(); i++) {
					// if in the right there is one variable that we do not sure is nullable, then we cannot sure whether the left is nullable
					if (nullable.find((*it)[i]) == nullable.end()) {
						is_nullable = 0;
					}
				}
				if (is_nullable) {
					// insert new nullable
					nullable.insert((*it)[0]);
					cont_loop = 1;
				}
			}
		}
	}
	//DEL_replacement_create_new

	for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
		std::vector<int32_t> nullable_index;
		nullable_index.clear();
		for (unsigned int i = 1; i < it->size(); i++) {
			if (nullable.find((*it)[i]) != nullable.end()) {
				nullable_index.push_back(i);
			}
		}
		if (!nullable_index.empty()) {
			std::vector<int> counter;
			counter.resize(nullable_index.size() + 1, 0);
			std::vector<int> skip_this_digit;

			while (counter[nullable_index.size()] == 0) {

				// calculate skip_this_digit
				skip_this_digit.clear();
				skip_this_digit.resize(it->size(), 0);
				for (unsigned int i = 0; i < nullable_index.size(); i++) {
					if (counter[i] == 1) {
						skip_this_digit[nullable_index[i]] = 1;
					}
				}
				// calculate tmp_rule and insert
				cfgrule tmp_rule;
				tmp_rule.clear();
				for (unsigned int i = 0; i < skip_this_digit.size(); i++) {
					if (skip_this_digit[i] == 0) {
						tmp_rule.push_back((*it)[i]);
					}
				}
				DELresult.insert(tmp_rule);

				// refresh counter (add 1)
				for (unsigned int i = 0; i < counter.size(); i++) {
					if (counter[i] == 1) {
						counter[i] = 0;
					} else {
						counter[i] = 1;
						i = counter.size();
					}
				}
			}
		} else {
			DELresult.insert(*it);
		}
	}
	//DEL_replacement_delete_nullable
	for (std::set<int32_t>::iterator it = nullable.begin(); it != nullable.end(); it++) {
		cfgrule tmp_rule;
		tmp_rule.resize(1);
		if (*it != s) {
			tmp_rule[0] = *it;
			DELresult.erase(tmp_rule);
		}
	}

	r = DELresult;
	DELresult.clear();
}
void cfg::to_CNF_UNIT() {
// UNIT: Eliminate unit rules
	std::set<cfgrule> removed;
	std::set<cfgrule> UNITresult;
	int UNIT_cont_loop = 1;
	while (UNIT_cont_loop) {
		UNIT_cont_loop = 0;
		// remove X -> X
		UNITresult.clear();
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (it->size() == 2) {
				if ((*it)[0] == (*it)[1] && 
					v.find((*it)[0]) != v.end() &&
					v.find((*it)[1]) != v.end() ) {
					// do not insert == remove
					UNIT_cont_loop = 1;
				} else {
					UNITresult.insert(*it);
				}
			} else {
				UNITresult.insert(*it);
			}
		}
		r = UNITresult;
		// find a rule X->Y to remove if there is a rule to remove
		std::set<cfgrule>::iterator to_remove = r.end();
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (it->size() == 2) {
				if (v.find((*it)[0]) != v.end() &&
					v.find((*it)[1]) != v.end() ) {
					to_remove = it;
				}
			}
		}
		if (to_remove != r.end()) {
			UNIT_cont_loop = 1;
			// if to_remove has already been done, don't add new rules.
			if (removed.find(*to_remove) == removed.end()) {
				// add to removed
				removed.insert(*to_remove);
				// add new rules
				int32_t to_remove_left = (*to_remove)[0];
				int32_t to_remove_right = (*to_remove)[1];
				for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
					if ((*it)[0] == to_remove_right) {
						cfgrule tmp_rule = *it;
						tmp_rule[0] = to_remove_left;
						UNITresult.insert(tmp_rule);
					}
				}
			}
			UNITresult.erase(*to_remove);
		}
		r = UNITresult;
	}
	return;
}

std::string cfg::description(int displaymode) {
// set variable to nonzero to display ascii
// but only integer version (displaymode = 0) can be used in checkcfgdescription();
// displaymode = 2 -> javascript
	std::string ret;
	char tmpInt32Char[11];
	std::string tmpInt32;	
	if (displaymode == 0) {
		ret += "V={";
		for (std::set<int32_t>::iterator it = v.begin(); it != v.end(); it++) {
			sprintf(tmpInt32Char, "%d", *it);
			tmpInt32 = tmpInt32Char;
			if (it != v.begin()) {
				ret += ",";
			}
			ret += tmpInt32;
		}
		ret += "}";
		ret += ";";
		ret += "Sigma={";
		for (std::set<int32_t>::iterator it = sigma.begin(); it != sigma.end(); it++) {
			sprintf(tmpInt32Char, "%d", *it);
			tmpInt32 = tmpInt32Char;
			if (it != sigma.begin()) {
				ret += ",";
			}
			ret += tmpInt32;
		}
		ret += "}";
		ret += ";";
		ret += "R={";
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (it != r.begin()) {
				ret += ",";
			}
			sprintf(tmpInt32Char, "%d", (*it)[0]);
			tmpInt32 = tmpInt32Char;
			ret += tmpInt32;
			ret += ":{";
			for (unsigned int itint = 1; itint < it->size(); itint++) {
				if (itint != 1) {
					ret += ",";
				}
				sprintf(tmpInt32Char, "%d", (*it)[itint]);
				tmpInt32 = tmpInt32Char;
				ret += tmpInt32;
			}
			ret += "}";
		}
		ret += "}";
		ret += ";";
		ret += "S=";
		sprintf(tmpInt32Char, "%d", s);
		tmpInt32 = tmpInt32Char;
		ret += tmpInt32;
	} else if (displaymode == 2) {
		ret += "V=[";
		for (std::set<int32_t>::iterator it = v.begin(); it != v.end(); it++) {
			sprintf(tmpInt32Char, "%d", *it);
			tmpInt32 = tmpInt32Char;
			if (it != v.begin()) {
				ret += ",";
			}
			ret += tmpInt32;
		}
		ret += "]";
		ret += ";\n";
		ret += "Sigma=[";
		for (std::set<int32_t>::iterator it = sigma.begin(); it != sigma.end(); it++) {
			sprintf(tmpInt32Char, "%d", *it);
			tmpInt32 = tmpInt32Char;
			if (it != sigma.begin()) {
				ret += ",";
			}
			ret += tmpInt32;
		}
		ret += "]";
		ret += ";\n";
		ret += "R=[";
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (it != r.begin()) {
				ret += ",";
			}
			ret += "[";
			for (unsigned int itint = 0; itint < it->size(); itint++) {
				if (itint != 0) {
					ret += ",";
				}
				sprintf(tmpInt32Char, "%d", (*it)[itint]);
				tmpInt32 = tmpInt32Char;
				ret += tmpInt32;
			}
			ret += "]";
		}
		ret += "]";
		ret += ";\n";
		ret += "S=";
		sprintf(tmpInt32Char, "%d", s);
		tmpInt32 = tmpInt32Char;
		ret += tmpInt32;
		ret += ";\n";
	} else if (displaymode == 1) {
		ret += "V={";
		for (std::set<int32_t>::iterator it = v.begin(); it != v.end(); it++) {
			if (it != v.begin()) {
				ret += ",";
			}
			ret.push_back((char)*it);
		}
		ret += "}";
		ret += ";";
		ret += "Sigma={";
		for (std::set<int32_t>::iterator it = sigma.begin(); it != sigma.end(); it++) {
			if (it != sigma.begin()) {
				ret += ",";
			}
			ret.push_back((char)*it);
		}
		ret += "}";
		ret += ";";
		ret += "R={";
		for (std::set<cfgrule>::iterator it = r.begin(); it != r.end(); it++) {
			if (it != r.begin()) {
				ret += ",";
			}
			ret.push_back((char)(*it)[0]);
			ret += ":{";
			for (unsigned int itint = 1; itint < it->size(); itint++) {
				if (itint != 1) {
					ret += ",";
				}
				ret.push_back((char)(*it)[itint]);
			}
			ret += "}";
		}
		ret += "}";
		ret += ";";
		ret += "S=";
		ret.push_back((char)s);
	}
	return ret;
}


