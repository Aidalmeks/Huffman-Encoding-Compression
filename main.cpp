#include <iostream>
#include "huffmancodes.h"

// 测试文件读写
#include <fstream>
#include <functional>
#include <string>

using namespace std;
// 计算文本文件的哈希值
size_t calculateHash(const string &filename)
{
	ifstream file(filename, ios::binary);
	if (!file.is_open())
	{
		cerr << "Error opening file: " << filename << endl;
		return 0;
	}

	// 从文件中读取内容
	string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

	// 使用std::hash计算哈希值
	size_t hashValue = hash<string>{}(content);

	file.close();
	return hashValue;
}

char xiehouyu[][100] = {{"指令无效!"}, {"我是有底线的哟ヽ（≧ □ ≦ ）ノ"}, {"不要在玩了,真的到底了( ′ 3`) sigh~"}};
int xuhao = 0;

int main()
{
	cout << "*********************************" << endl;
	cout << "***基于Huffman编码的文档解压缩***" << endl;
	cout << "***      欢迎使用本小程序!    ***" << endl;
	cout << "***   请选择您要使用的功能：  ***" << endl;
	cout << "***        1.压缩文件         ***" << endl;
	cout << "***        2.解压文件         ***" << endl;
	cout << "***  3.压缩前后的文件是否一致 ***" << endl;
	cout << "***  4.hash判断文件是否一致   ***" << endl;
	cout << "***        0.关闭程序         ***" << endl;
	cout << "*********************************" << endl;

Tiaochu:
	cout << "等待您的指令输入：";
	string option;
	cin >> option;

	if (option.compare("1") == 0)
	{
		cout << "请输入要压缩的文件名：";
		string compress_filename;
		cin >> compress_filename;
		zfish::HuffmanCodes hec{compress_filename, "", false}; // hex表示之后的数字以16进制方式输出
		hec.run();
		goto Tiaochu;
	}
	else if (option.compare("2") == 0)
	{
		cout << "请输入要解压的文件名：";
		string uncompress_filename;
		cin >> uncompress_filename;
		cout <<"请输入对应的频率记录文件";
		string frequencyfile;
		cin >> frequencyfile;

		zfish::HuffmanCodes hec{uncompress_filename, frequencyfile, true}; // hex表示之后的数字以16进制方式输出
		hec.run();
		goto Tiaochu;
	}
	else if (option.compare("3") == 0)
	{
		string filename1;
		string filename2;
		cout << "请输入原文件名: ";
		cin >> filename1;
		cout << "请输入解压后的文件名: ";
		cin >> filename2;

		zfish::HuffmanCodes sign(filename1,"", false); // 创建 HuffmanCodes 对象
		bool filesEqual = sign.equalFile(filename1, filename2);
		cout << "Files are " << (filesEqual ? "equal" : "not equal") << endl;
	}
	else if (option.compare("0") == 0)
	{
		cout << "感谢您的使用!" << endl;
		exit(0);
	}
	else if (option.compare("4") == 0)
	{
		cout << "使用哈希表判断文件是否相等" << endl;
		string filename1;
		string filename2;
		cout << "请输入原文件名: ";
		cin >> filename1;
		cout << "请输入解压后的文件名: ";
		cin >> filename2;

		bool sign1 = calculateHash(filename1);
		bool sign2 = calculateHash(filename2);

		cout << "Files are " << ((sign1 == true) && (sign2 == true) ? "equal" : "not equal") << endl;
	}

	else if (option.compare("1") || option.compare("2"))
	{
		cout << "请输入数字1或2或3或0,";

		// 无聊一下
		for (int i = 0; i < 100; i++)
		{
			cout << xiehouyu[xuhao][i];
		}
		cout << endl;

		if (xuhao < 2)
		{
			++xuhao;
		}
		else
		{
			xuhao = 0;
		}

		goto Tiaochu;
	}

	system("pause");
	return 0;
}
