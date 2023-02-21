#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

#define isNum(c) (isdigit(c)?c-48:(c=='E'?10:(c=='.'?11:(c=='-'?12:(c=='+'?13:-1)))))

double str2num(string s)
{	//字符串转数字，包括整数、小数和科学记数法 
	int i, j, k, negative = 0;
	double n = 0;
	string s1, s2;

	if (s.empty()) return 0;
	if (s[0] == '-') negative = 1; //设置负数标记 
	if (s[0] == '+' || s[0] == '-') s = s.substr(1, s.size());
	//--------------- 
	for (i = 0; i < s.size(); i++) //排除不需要的字符 
		if (isNum(s[i]) == -1) return pow(-1.1, 1.1);
	// if (s[0] == 'E' || s[0] == '.' || s[s.size() - 1] == 'E' || s[s.size() - 1] == '.') //排除 E或. 出现在首尾 
	if (s[0] == 'E' || s[0] == '.' || s[s.size() - 1] == 'E')
		return pow(-1.1, 1.1); //排除E出现在首尾 以及 .出现在首部（inp文件的特殊要求）
	i = -1; j = 0;
	while ((i = int(s.find('.', ++i))) != s.npos) j++;
	if (j > 1) return pow(-1.1, 1.1); //排除多个小数点 
	i = -1; j = 0;
	while ((i = int(s.find('E', ++i))) != s.npos) j++;
	if (j > 1) return pow(-1.1, 1.1); //排除多个字母E
	if (s.find('E') == s.npos) //没有E时排除加减
		if (s.find('+') != s.npos || s.find('-') != s.npos) return pow(-1.1, 1.1);
	//---------------
	if ((i = int(s.find('E'))) != s.npos) {
		s1 = s.substr(0, i); //尾数部分 
		s2 = s.substr(i + 1, s.size()); //阶码 
		if (s2[0] == '+') s2 = s2.substr(1, s2.size()); //阶码为正数，去掉+ 
		if (s2.find('.') != s2.npos) return pow(-1.1, 1.1); //阶码不准出现小数
		n = str2num(s1) * pow(10.0, str2num(s2)); //尾数和阶码分别递归调用 
		return negative ? -n : n;
	}
	i = 0; k = 1;
	if ((i = int(s.find('.'))) != s.npos) {
		if (s.find('.') == s.length()) {
			n += str2num(s.substr(0, i - 1));
		}
		for (j = i + 1; j < s.length(); j++, k++)
			n += isNum(s[j]) / pow(10.0, (double)k);
		n += str2num(s.substr(0, i));  //整数部分递归调用 
	}
	else
		for (j = 0; j < s.size(); j++)
			n = n * 10 + isNum(s[j]);

	return negative ? -n : n; //负数返回-n 
}

int main(void)
{
	vector<string>a = { "-12.","0.","+12.345","123456789012345","1.23456789012345E+20",
		"-1.5E-2","1E2","3E1.1","1.7977E+308","-4.95E-324","1.1.1","1E2E2","abc","1+2" };
	for (auto s : a) {
		cout << s << "->" << setprecision(15) << float(str2num(s)) << endl;
	}
}