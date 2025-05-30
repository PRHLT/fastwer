#include "fastwer.hpp"


void fastwer::tokenize(const string &str, vector<string> &tokens, bool char_level, char delim) {
    stringstream ss(str);
    string token;
    if (char_level) {
        //for (char c : str) { tokens.push_back(string(1, c)); }
	// For UTF-8: https://www.appsloveworld.com/cplus/100/126/c-iterate-utf-8-string-with-mixed-length-of-characters
	// By AHT
	for(size_t i = 0; i < str.length();) {
    	  int cplen = 1;
    	  if ((str[i] & 0xf8) == 0xf0) cplen = 4;
          else if ((str[i] & 0xf0) == 0xe0) cplen = 3;
          else if ((str[i] & 0xe0) == 0xc0) cplen = 2;
          if ((i + cplen) > str.length()) cplen = 1;
          tokens.push_back(str.substr(i, cplen));
          i += cplen;
        }
    } else {
        while (getline(ss, token, delim)) { tokens.push_back(token); }
    }
}

double fastwer::round_to_digits(double d, uint8_t digits) {
    assert(digits < 7);
    static double pow10[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
    return round(d * pow10[digits]) / pow10[digits];
}

pair <uint32_t, uint32_t> fastwer::compute(string &hypo, string &ref, bool char_level) {
    vector<string> hypo_tokens, ref_tokens;
    fastwer::tokenize(hypo, hypo_tokens, char_level);
    fastwer::tokenize(ref, ref_tokens, char_level);

    uint32_t m = hypo_tokens.size() + 1, n = ref_tokens.size() + 1;
    vector<uint32_t> f(m * n);
    for (size_t i = 0; i < m; i++) { f[i * n] = i; }
    for (size_t j = 0; j < n; j++) { f[0 * n + j] = j; }
    for (size_t i = 1; i < m; i++) {
        for (size_t j = 1; j < n; j++) {
            f[i * n + j] = min(f[(i - 1) * n + j] + 1, f[i * n + (j - 1)] + 1);
            uint32_t matching_case = f[(i - 1) * n + (j - 1)] + (hypo_tokens[i - 1] == ref_tokens[j - 1] ? 0 : 1);
            f[i * n + j] = min(f[i * n + j], matching_case);
        }
    }
    return make_pair(f[m * n - 1], ref_tokens.size());
}

// By AHT
// Implement bag-of-words algorithm
tuple<uint32_t, uint32_t, uint32_t> fastwer::bagOfWords(string &hypo, string &ref, bool char_level) {
    vector<string> hypo_tokens, ref_tokens;
    fastwer::tokenize(hypo, hypo_tokens, char_level);
    fastwer::tokenize(ref, ref_tokens, char_level);

    uint32_t m = hypo_tokens.size(), n = ref_tokens.size(), acc = 0;
    unordered_map<string, int32_t> hst, hstBck;
    hst.reserve(700);

    for (size_t j = 0; j < n; j++) {
    	hstBck[ref_tokens[j]]++; hst[ref_tokens[j]]++;
    }
    for (size_t i = 0; i < m; i++) hst[hypo_tokens[i]]--;

    for (const auto x: hstBck)
    	acc += (hst[x.first]>0)?x.second-hst[x.first]:x.second;

    uint32_t err = 0;
    for (const auto x: hst) err += abs(x.second);
    
    return make_tuple(err, acc, n);
}

// By AHT
// Return also the Dynamic-Programming sequence alignment
tuple<list<tuple <int16_t, int16_t, uint8_t>>, uint32_t, uint32_t> fastwer::alignment(string &hypo, string &ref, bool char_level) {
    vector<string> hypo_tokens, ref_tokens;
    fastwer::tokenize(hypo, hypo_tokens, char_level);
    fastwer::tokenize(ref, ref_tokens, char_level);

    uint32_t m = hypo_tokens.size() + 1, n = ref_tokens.size() + 1;
    vector<uint32_t> f(m * n);
    for (size_t i = 0; i < m; i++) { f[i * n] = i; }
    for (size_t j = 0; j < n; j++) { f[0 * n + j] = j; }
    for (size_t i = 1; i < m; i++) {
        for (size_t j = 1; j < n; j++) {
            f[i * n + j] = min(f[(i - 1) * n + j] + 1, f[i * n + (j - 1)] + 1);
            uint32_t matching_case = f[(i - 1) * n + (j - 1)] + (hypo_tokens[i - 1] == ref_tokens[j - 1] ? 0 : 1);
            f[i * n + j] = min(f[i * n + j], matching_case);
        }
    }

    list<tuple <int16_t, int16_t, uint8_t>> lst;
    int16_t i = m-1, j = n-1;
    while (i != 0 || j != 0) {
  	if ( i > 0 && j > 0 && f[i * n + j] == f[(i - 1) * n + (j - 1)] + (hypo_tokens[i - 1] == ref_tokens[j - 1] ? 0 : 1) ) 
		lst.push_front(make_tuple(--i,--j,(hypo_tokens[i - 1] == ref_tokens[j - 1] ? 0 : 1)));
	else if ( i > 0 && f[i * n + j] == f[(i - 1) * n + j] + 1 ) 
	  	lst.push_front(make_tuple(--i,-1,1));
	else if ( j > 0 && f[i * n + j] == f[i * n + (j - 1)] + 1)
		lst.push_front(make_tuple(-1,--j,1));
    }
	
    return make_tuple(lst, f[m * n - 1], ref_tokens.size());
}

double fastwer::score_sent(string &hypo, string &ref, bool char_level) {
    pair<uint32_t, uint32_t> stats = fastwer::compute(hypo, ref, char_level);
    return fastwer::round_to_digits(100 * double(stats.first) / stats.second, 4);
}

double fastwer::score(vector<string> &hypo, vector<string> &ref, bool char_level) {
    size_t n_examples = hypo.size();
    assert(n_examples == ref.size());
    vector<pair<uint32_t, uint32_t>> stats(n_examples);
    double total_edits = 0.0, total_lengths = 0.0;
    for (size_t i = 0; i < n_examples; i++) {
        stats.push_back(fastwer::compute(hypo[i], ref[i], char_level));
        total_edits += stats.back().first;
        total_lengths += stats.back().second;
    }
    return fastwer::round_to_digits(100 * total_edits / total_lengths, 4);
}
