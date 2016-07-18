#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <cstdlib> 
#include <vector>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <stdio.h>

using namespace std;

#define MIN_VALUE 4.9E-324
#define MAX_VALUE 1.7976931348623157E308
#define LOG0 -1.7976931348623157E308
#define LOG2 0.69314718055

typedef vector<vector<double> > doublevecvec;

class Span
{
public:
	int id;
	int start;
	int end;
	string label;

public:
	Span() {id = 0; start = -1; end= -1; label = ""; }
	Span(int s, int e, string l="") {id = 0; start = s; end = e; label = l; }

	bool intersect(Span en)
	{
		if(isEqual(en))
			return false;
		if(overlap(en) > 0)
			return true;
		return false;
	}

	int overlap(Span en)
	{
		int n = 0;
		for(int i=start; i<=end; i++) {
			if(i>=en.start && i<=en.end)
				n += 1;
		}
		return n;
	}

	bool isEqual(Span en)
	{
		if(start == en.start && end == en.end)
			return true;
		else
			return false;
	}

	bool isContain(int i)
	{
		if(i>=start && i<=end)
			return true;
		else
			return false;
	}

	bool Equal(int _start, int _end) {
		return (start == _start && end == _end);
	}
	int len() {if(start == -1 || end == -1) return 0; else return end-start+1;}
	bool inRange(int s, int e)
	{
		if(s<=start && e>=end)
			return true;
		return false;
	}

};

class Utils
{
public:
	Utils();
public:
	static set<string> SentenceTags;
	static set<string> PhraseTags;
	static set<string> WordTags;
public:

	static set<string> InitializeSentTags();
	static set<string> InitializeWordTags();
	static set<string> InitializePhraseTags();

	static int diff_segment(int start1, int end1, int start2, int end2);

	static void addElem(map<int, vector<string> >& pmap, int c, string str);

	template <class A>
	static void shuffle(vector<A>& indicies) {
		std::srand ( unsigned ( time(0) ) );
		// using built-in random generator:
		std::random_shuffle( indicies.begin(), indicies.end());
	}

	static void capitalize_word(string &word);
	static void capitalize(char &ch);

	static void split_inorder(int n, vector<double> ratio, map<int, int>& id2part);

	static void Split(string& strSrc,const char chDelim,vector<string>& vecDes);

	static string getValue(vector<string> fields, string key, string delim);
	static void getFVpair(string str, pair<string, string>& fvpair, string delim);

	static void Strip(string &line);

	static string toLowerString(string line);

	static void add(doublevecvec& a, doublevecvec& b);

	static void add(vector<double>& a, vector<double>& b);

	static void initialize(doublevecvec& a);

	static void Trim(string &line);

	static string int2string(int num);
	static string double2string(double v);

	static bool isPunctuation(string str);
	static bool isNum(string str);
	static bool isInteger(string str);

	static string toLower(string str);
	
	template <class A, class B>
	static bool increase_first(pair<A, B> p1, pair<A, B> p2){ return p1.first<p2.first;}
	template <class A, class B>
	static bool decrease_second(pair<A, B> a1, pair<A, B> a2) {return a1.second>a2.second;}
	template <class A, class B>
	static bool decrease_first(pair<A, B> a1, pair<A, B> a2) {return a1.first>a2.first;}
	template <class A, class B>
	static bool increase_second(pair<A, B> a1, pair<A, B> a2) {return a1.second<a2.second;}

	static void loadiiMap(string filename, map<int, int>& idmap);
	static void loadsiMap(string filename, map<string, int>& idmap);
	static void WriteWordMap(map<string, int> &wordmap, string wordmapfile);
	static void ReadWordMap(map<string, int> &wordmap, string wordmapfile);

	static vector<Span> getSpans(vector<string> tags, string scheme);

	static vector<pair<string, double> > sortMap(map<string, double>& pmap, bool increase=true);
	static vector<pair<string, int> > sortMap(map<string, int>& pmap, bool increase=true);
	static char* itoa(int value, char* result, int base);

	static string feature2str(map<int, double> features);

	static void printvecvec(vector<vector<double> > v);
	static void printvec(double* v, int size);
	static void printvec(vector<double> v);

	static bool toLower(std::string *s);

	static void loadCRFdata(string infilename, vector<vector<string> >& crfdata);

	//static void ReadFileInDir(string data_path, string key, vector<string> &filenames);

};
