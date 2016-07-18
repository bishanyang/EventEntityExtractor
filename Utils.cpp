#include "Utils.h"
#include <sstream>

set<string> Utils::SentenceTags = Utils::InitializeSentTags();
set<string> Utils::PhraseTags = Utils::InitializePhraseTags();
set<string> Utils::WordTags = Utils::InitializeWordTags();

set<string> Utils::InitializeWordTags()
{
	set<string> WordTags;

	WordTags.insert("CC");
	WordTags.insert("CD");
	WordTags.insert("DT");
	WordTags.insert("EX");
	WordTags.insert("FW");
	WordTags.insert("IN");
	WordTags.insert("JJ");
	WordTags.insert("JJR");
	WordTags.insert("JJS");
	WordTags.insert("LS");
	WordTags.insert("MD");
	WordTags.insert("NN");
	WordTags.insert("NNS");
	WordTags.insert("NNP");
	WordTags.insert("NNPS");
	WordTags.insert("PDT");
	WordTags.insert("POS");
	WordTags.insert("PRP");
	WordTags.insert("PRP$");
	WordTags.insert("RB");
	WordTags.insert("RBR");
	WordTags.insert("RBS");
	WordTags.insert("RP");
	WordTags.insert("SYM");
	WordTags.insert("TO");
	WordTags.insert("UH");
	WordTags.insert("VB");
	WordTags.insert("VBD");
	WordTags.insert("VBG");
	WordTags.insert("VBN");
	WordTags.insert("VBP");
	WordTags.insert("VBZ");
	WordTags.insert("WDT");
	WordTags.insert("WP");
	WordTags.insert("WP$");
	WordTags.insert("WRB");

	return WordTags;
}

set<string> Utils::InitializePhraseTags()
{
	set<string> PhraseTags;

	PhraseTags.insert("ADJP");
	PhraseTags.insert("ADVP");
	PhraseTags.insert("CONJP");
	PhraseTags.insert("PP");
	PhraseTags.insert("VP");
	PhraseTags.insert("NP");
	PhraseTags.insert("WHADJP");
	PhraseTags.insert("WHAVP");
	PhraseTags.insert("WHNP");
	PhraseTags.insert("WHPP");
	PhraseTags.insert("FRAG");
	PhraseTags.insert("INTJ");
	PhraseTags.insert("LST");
	PhraseTags.insert("NAC");
	PhraseTags.insert("NX");
	PhraseTags.insert("PRN");
	PhraseTags.insert("PRT");
	PhraseTags.insert("QP");
	PhraseTags.insert("RRC");
	PhraseTags.insert("UCP");
	PhraseTags.insert("X");

	return PhraseTags;
}

set<string> Utils::InitializeSentTags()
{
	set<string> SentenceTags;

	SentenceTags.insert("ROOT");
	SentenceTags.insert("S");
	SentenceTags.insert("SBAR");
	SentenceTags.insert("SBARQ");
	SentenceTags.insert("SINV");
	SentenceTags.insert("SQ");

	return SentenceTags;
}

bool Utils::toLower(std::string *s) {
  for (size_t i = 0; i < s->size(); ++i) {
    char c = (*s)[i];
    if ((c >= 'A') && (c <= 'Z')) {
      c += 'a' - 'A';
      (*s)[i] = c;
    }
  }
  return true;
}
string Utils::getValue(vector<string> fields, string key, string delim)
{
	pair<string, string> fvpair;
	int i = 0;
	for(i=0; i<fields.size(); i++)
	{
		getFVpair(fields[i], fvpair, delim);
		if(fvpair.first == key)
			break;
	}
	if(i<fields.size())
		return fvpair.second;
	else
		return "";
}

void Utils::getFVpair(string str, pair<string, string>& fvpair, string delim)
{
	int index = str.find(delim);
	if(index == -1)
		return;
	fvpair.first = str.substr(0,index);
	fvpair.second = str.substr(index+delim.size());
}

bool Utils::isPunctuation(string str)
{
	if(str == "." || str =="," || str =="''" || str =="``"|| str == "--"
			|| str ==":" || str =="?" || str =="!" || str ==";" || str == "-LRB-" || str == "-RRB-" || str == "-")
		return true;
	else
		return false;
}

void Utils::addElem(map<int, vector<string> >& pmap, int c, string str)
{
	if(pmap.find(c) == pmap.end())
	{
		vector<string> v;
		v.push_back(str);
		pmap[c] = v;
	}
	else
	{
		pmap[c].push_back(str);
	}
}

char* Utils::itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

string Utils::int2string(int num)
{
	char ch[100];
	itoa(num, ch, 10);
	string str = ch;
	return str;
}

string Utils::double2string(double v)
{
	//char buffer[1024];
	//sprintf(buffer, "%.7lf", v);
	//string str = buffer;
	//return str;
	std::ostringstream ss;
	ss << v;
	return ss.str();
}
void Utils::Split(string& strSrc,const char chDelim,vector<string>& vecDes)
{
	Trim(strSrc); //remove \r,\n,' '

	basic_string<char>::size_type pos, current = 0, npos =(basic_string<char>::size_type)-1;

	vecDes.clear();
	while (npos != (pos = strSrc.find(chDelim,current)))
	{
		if (pos != current)
		{//prevent empty string be put into vector
			vecDes.push_back(strSrc.substr(current,pos - current));
		}
		current = pos + 1;		
	}
	if (current != strSrc.length())
	{
		vecDes.push_back(strSrc.substr(current,strSrc.length() - current)); //care about the last one is nothing
	}
}

void Utils::Strip(string &line)
{
    int length = line.length();
    if(length > 0 && (line[length - 1] == '\r'||line[length - 1] == '\n'))
        line = line.substr(0, length - 1);
}

void Utils::Trim(string &line)
{
    int i = 0;
	int j = (int)line.size()-1;
	while(line[i] == ' ' || line[i] == '\r' || line[i] == '\n') i++;
	while(line[j] == ' ' || line[j] == '\r' || line[j] == '\n') j--;
	line = line.substr(i, j-i+1);
}

string Utils::toLowerString(string line)
{
    string result = "";
    for(int i = 0; i < line.length(); i++)
        result += tolower(line[i]);
    return result;
}

void Utils::add(doublevecvec& a, doublevecvec& b)
{
	if((int)a.size() != (int)b.size())
		return;
	if((int)a.size() > 0 && (int)a[0].size() != (int)b[0].size())
		return;

	for(int i=0; i<(int)a.size(); i++)
	{
		for(int j=0; j<(int)a[i].size(); j++)
			a[i][j] += b[i][j];
	}
}

void Utils::add(vector<double>& a, vector<double>& b)
{
	if((int)a.size() != (int)b.size())
		return;
	
	for(int i=0; i<(int)a.size(); i++)
		a[i] += b[i];
}

void Utils::initialize(doublevecvec& a)
{
	for(int i=0; i<(int)a.size(); i++)
	{
		for(int j=0; j<(int)a[i].size(); j++)
			a[i][j] = 0;
	}
}

bool Utils::isNum(string str)
{
	for(int i=0; i<str.size(); i++)
	{
		if((str[i] >= '0' && str[i] <='9') || str[i] == ',' || str[i] == '.')
			continue;
		else
			return false;
	}
	return true;
}

bool Utils::isInteger(string str)
{
	for(int i=0; i<str.size(); i++)
	{
		if(str[i] >= '0' && str[i] <='9')
			continue;
		else
			return false;
	}
	return true;
}

string Utils::toLower(string str)
{
	string newstr = "";
	for(int i=0; i<str.size(); i++)
	{
		if(str[i] >= 'A' && str[i] <= 'Z')
			newstr += str[i]-'A'+'a';
		else
			newstr += str[i];
	}
	return newstr;
}

int Utils::diff_segment(int start1, int end1, int start2, int end2)
{//1st contained in 2nd
	int diff = 0;
	if(start1 < start2)
		diff = start2-start1;
	if(end1 > end2)
		diff = max(diff, end1-end2);
	return diff;
}
vector<pair<string, int> > Utils::sortMap(map<string, int>& pmap, bool increase)
{
	vector<pair<string, int> > vec(pmap.begin(), pmap.end());
	if(increase)
		sort(vec.begin(), vec.end(), Utils::increase_second<string, int>);
	else
		sort(vec.begin(), vec.end(), Utils::decrease_second<string, int>);
	return vec;
}

vector<pair<string, double> > Utils::sortMap(map<string, double>& pmap, bool increase)
{
	vector<pair<string, double> > vec(pmap.begin(), pmap.end());
	if(increase)
		sort(vec.begin(), vec.end(), Utils::increase_second<string, double>);
	else
		sort(vec.begin(), vec.end(), Utils::decrease_second<string, double>);
	return vec;
}

string Utils::feature2str(map<int, double> features)
{
	string fstr = "";
	map<int, double>::iterator iter;
	vector<pair<int, double> > fvec;
	for(iter=features.begin(); iter!=features.end(); iter++)
		fvec.push_back(make_pair(iter->first, iter->second));
	sort(fvec.begin(), fvec.end(), Utils::increase_first<int, double>);

	for(int i=0; i<fvec.size(); i++) {
		fstr += Utils::int2string(fvec[i].first+1)+":"+Utils::double2string(fvec[i].second)+" ";
	}
	return fstr;
}

void Utils::printvecvec(vector<vector<double> > v)
{
	cout<<"print"<<endl;
	for(int i=0; i<v.size(); i++)
	{
		for(int j=0; j<v[i].size(); j++)
		{
			if(v[i][j] == LOG0)
				cout<<"LOG0"<<" ";
			else
				cout<<v[i][j]<<" ";
		}
		cout<<endl;
	}
}
void Utils::printvec(double* v, int size)
{
	cout<<"print"<<endl;
	for(int i=0; i<size; i++)
		cout<<v[i]<<" ";
	cout<<endl;
}
void Utils::printvec(vector<double> v)
{
	cout<<"print"<<endl;
	for(int i=0; i<v.size(); i++)
		cout<<v[i]<<" ";
	cout<<endl;
}
void Utils::loadCRFdata(string infilename, vector<vector<string> >& crfdata)
{
  ifstream infile(infilename.c_str(), ios::in);
  string str;
  vector<string> lines;
  while(getline(infile, str))
  {
	  if(str == "")
	  {
		  crfdata.push_back(lines);
		  lines.clear();
	  }
	  else
		  lines.push_back(str);
  }
  if(str != "")
	  crfdata.push_back(lines);
}

void Utils::capitalize(char &ch) {
	if (ch >= 'a' && ch <= 'z') {
		ch = 'A' + ch - 'a';
	}
}

void Utils::capitalize_word(string &word) {
	for (int i = 0; i < word.size(); ++i) {
		if (word[i] >= 'a' && word[i] <= 'z') {
			word[i] = 'A' + word[i] - 'a';
		}
	}
}

void Utils::split_inorder(int n, vector<double> ratio, map<int, int>& id2part)
{
	int index = 0;
	int c = 0;
	for(c = 0; c<ratio.size(); c++)
	{
		int subn = n*ratio[c];
		for(int i=0; i<subn; i++)
			id2part[index+i] = c;
		index += subn;
	}
	//leftover
	c--;
	for(;index<n; index++)
		id2part[index] = c;
}

void Utils::loadiiMap(string filename, map<int, int>& idmap)
{
	idmap.clear();
	ifstream infile(filename.c_str(), ios::in);
	string str;
	while(getline(infile, str))
	{
		vector<string> fields;
		Utils::Split(str, ' ', fields);
		idmap[atoi(fields[0].c_str())] = atoi(fields[1].c_str());
	}
	infile.close();
}

void Utils::loadsiMap(string filename, map<string, int>& idmap)
{
	idmap.clear();
	ifstream infile(filename.c_str(), ios::in);
	string str;
	while(getline(infile, str)) {
		vector<string> fields;
		Utils::Split(str, '\t', fields);
		idmap[fields[0]] = atoi(fields[1].c_str());
	}
	infile.close();
}

void Utils::WriteWordMap(map<string, int> &wordmap, string wordmapfile) {
	vector<pair<int, string> > word_vec;
	ofstream mapfile(wordmapfile.c_str(), ios::out);
	for(map<string, int>::iterator iter = wordmap.begin(); iter != wordmap.end(); ++iter) {
		word_vec.push_back(make_pair(iter->second, iter->first));
	}
	sort(word_vec.begin(), word_vec.end(), Utils::increase_first<int, string>);
	for (int i = 0; i <word_vec.size(); ++i) {
		mapfile<<word_vec[i].second<<endl;
	}
	mapfile.close();
}

void Utils::ReadWordMap(map<string, int> &wordmap, string wordmapfile) {
	wordmap.clear();
	ifstream mapfile(wordmapfile.c_str(), ios::in);
	string str;
	int word_id = 0;
	while(getline(mapfile, str)) {
		wordmap[str] = word_id++;
	}
	mapfile.close();
}

vector<Span> Utils::getSpans(vector<string> tags, string scheme)
{
	vector<Span> spans;
	if (scheme == "BIO") {
		int i = 0;
		while(i<tags.size()) {
			if(tags[i].substr(0,2) == "B_") {
				int start = i;
				string label = tags[i].substr(2);
				i += 1;
				while(i<tags.size() && tags[i] == label) i++;
				Span en(start, i-1, label);
				spans.push_back(en);
			}
			else
				i += 1;
		}
	} else if (scheme == "IO") {
		int i = 0;
		while(i<tags.size()) {
			if(tags[i] != "O") {
				int start = i;
				string label = tags[i];
				i += 1;
				while(i<tags.size() && tags[i] == label) i++;
				Span en(start, i-1, label);
				spans.push_back(en);
			}
			else
				i += 1;
		}
	}
	return spans;
}
