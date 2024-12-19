#ifndef HUFFMANCODES_H
#define HUFFMANCODES_H

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#define ULL unsigned long long
#define Byte unsigned char

namespace zfish
{

	// 码数
	constexpr int kCodeNum = 256;
	// 压缩识别符长度
	constexpr int kLenOfZipName = 16;
	// 文件名长度
	constexpr int kLenOfFileName = 256;
	// 文件大小值长度
	constexpr int kLenOfFileSize = 8;
	// 码频值长度
	constexpr int kLenOfCodeFrequency = 8;
	// 头文件信息长度
	constexpr int kLenOfZipHeader = kLenOfZipName + kLenOfFileName +
									kLenOfFileSize + kLenOfCodeFrequency * kCodeNum;
	// 压缩 识别符
	static const char kZipName[kLenOfZipName] = "zzzfish";

	// 码点
	struct HuffmanCodePoint
	{
		Byte old_code;			  // 旧码
		ULL frequency;			  // 频率
		ULL new_code;			  // 新码
		int length;				  // 新码长度
		std::string new_code_str; // 新码字符串表示（debug用）

		HuffmanCodePoint() : old_code(0), frequency(0), new_code(0), length(0) {}
	};

	// Huffman 节点
	struct HuffmanTreeNode
	{
		ULL weight;				 // 权重
		HuffmanCodePoint *point; // 指向码点（叶子节点才有指向）
		HuffmanTreeNode *left;	 // 左分支
		HuffmanTreeNode *right;	 // 右分支

		HuffmanTreeNode(ULL weight = 0, HuffmanCodePoint *point = nullptr,
						HuffmanTreeNode *left = nullptr,
						HuffmanTreeNode *right = nullptr)
			: weight(weight), point(point), left(left), right(right) {}
	};

	// 自定义比较器，用于优先队列
	struct CmparatorOfHuffmanTreeNode
	{
		bool operator()(HuffmanTreeNode *&lhs, HuffmanTreeNode *&rhs) const
		{
			return lhs->weight > rhs->weight; // 小根堆
		}
	};

	//读取结构
	struct HuffmanInfo {
    int originalValue;
    int frequency;
    std::string huffmanCode;
    int length;
    uint64_t decimalValue;

    HuffmanInfo(int orig, int freq, const std::string& code, int len, uint64_t dec)
        : originalValue(orig), frequency(freq), huffmanCode(code), length(len), decimalValue(dec) {}
	};
	
	class HuffmanCodes
	{
	public:
		HuffmanCodes(const std::string &inputFileName, const std::string &frequencyFileName, bool isCompress);
		~HuffmanCodes();
			//读取frequencyOutput函数
std::vector<HuffmanInfo> loadHuffmanData(const std::string& filename) {
    std::ifstream inputFile(filename);
    std::vector<HuffmanInfo> huffmanData;

    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return huffmanData;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        std::istringstream iss(line);
        int originalValue, frequency, length;
        std::string huffmanCode;
        uint64_t decimalValue;

        if (iss >> originalValue >> frequency >> huffmanCode >> length >> decimalValue) {
            huffmanData.emplace_back(originalValue, frequency, huffmanCode, length, decimalValue);
        }
    }

    inputFile.close();
    return huffmanData;
}
		// 禁止拷贝构造
		HuffmanCodes(const HuffmanCodes &) = delete;
		HuffmanCodes &operator=(const HuffmanCodes &) = delete;

		/// 如果是未压缩的，则进行压缩，如果是压缩，则解压缩
		void run();

		// 统计频率
		void statisticalFrequency();

		// 构建 Huffman 树
		HuffmanTreeNode *buildHuffmanTree();

		// 获得 Huffman 编码
		void initCodePoint(HuffmanTreeNode *node, ULL new_code,
						   std::string new_code_str, int length);
		

		// 压缩文件
		void compress();

		// 搜索节点，辅助于解码
		bool findNode(HuffmanTreeNode *&node, Byte inputByte, int &pos);

		// 解压缩
		void uncompress();

		// 打印哈夫曼编码信息
		void printHuffmanEncodeInfo(const std::string &outputFileName);

		// 打印压缩或解压缩信息
		void printInfo(const char *type);

		// 获取输出文件名
		std::string getOutputFileName() const
		{
			return output_filename_;
		}

		// 判断两个文件是否相等
		bool equalFile(const std::string &fileName1, const std::string &fileName2);
	private:
		// 释放节点
		void freeNode(HuffmanTreeNode *node);
		std::string frequency_filename_;//参考频率率参数

		HuffmanCodePoint points_[kCodeNum]; // 码
		HuffmanTreeNode *root_;				// Huffman树根节点
		bool is_compressed_;				// 是否压缩（用于多次压缩 ）

		std::string input_filename_;  // 输入文件名
		std::string output_filename_; // 输出文件名
		ULL input_filesize_;		  // 输入文件大小
		ULL output_filesize_;		  // 输出文件大小
	};
	
}
#endif
