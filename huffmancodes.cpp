#include "huffmancodes.h"

#include <cstdio>
#include <cstring>
#include <queue>
#include <iostream>
// 测试文件读写
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
using namespace zfish;

HuffmanCodes::HuffmanCodes(const std::string &inputFileName, const std::string &frequencyFileName, bool isCompress)
	: root_(nullptr),
	  is_compressed_(isCompress),
	  input_filename_(inputFileName),
	  frequency_filename_(frequencyFileName),
	  output_filename_(inputFileName + ".zLzip"),
	  input_filesize_(0),
	  output_filesize_(0)
{
	for (int i = 0; i < kCodeNum; ++i)
	{
		points_[i].old_code = i;
	}
}

// 释放节点及其子树
void HuffmanCodes::freeNode(HuffmanTreeNode *node)
{
	if (node)
	{
		freeNode(node->left);
		freeNode(node->right);
		delete node;
	}
}

// 析构
HuffmanCodes::~HuffmanCodes()
{
	freeNode(root_);
}

/// 如果是未压缩的，则进行压缩，如果是压缩，则解压缩
void HuffmanCodes::run()
{
    if (frequency_filename_.empty())
    {
        FILE *input_fp = nullptr;
        if ((input_fp = fopen(input_filename_.c_str(), "rb")) == nullptr)
        {
            printf("open file %s failed!\n", input_filename_.c_str());
            exit(-1);
        }

        char zip_name[kLenOfZipName];
        fread(zip_name, kLenOfZipName, 1, input_fp);

        if (is_compressed_ || strcmp(zip_name, kZipName))
        {
            // 无识别符，非压缩文件
            fclose(input_fp);
            printf("开始压缩文件%s......\n", input_filename_.c_str());
            printf("正在统计频率......\n");
            statisticalFrequency();
            printf("正在构建哈夫曼树......\n");
            root_ = buildHuffmanTree();
            printf("正在产生新编码......\n");
            initCodePoint(root_, 0, string(), 0);
            printHuffmanEncodeInfo("_rcd.txt");
            printf("正在压缩......\n");
            compress(); // 压缩
            printInfo("压缩");
            printf("压缩成功\n");
        }
        fclose(input_fp);
    }
    else {
    // 使用 loadHuffmanData 读取频率数据
    std::vector<HuffmanInfo> huffmanData = loadHuffmanData(frequency_filename_);
    if (huffmanData.empty()) {
        cerr << "Error: Failed to load Huffman data from " << frequency_filename_ << endl;
        return;
    }

    // 根据读取的数据设置 points_ 数组
    for (const auto& info : huffmanData) {
        points_[info.originalValue].frequency = info.frequency;
        points_[info.originalValue].new_code_str = info.huffmanCode;
        points_[info.originalValue].length = info.length;
        // 可能需要将 info.decimalValue 转换为合适的格式
        points_[info.originalValue].new_code = info.decimalValue;
    }

    printf("开始解压缩文件%s......\n", input_filename_.c_str());
    printf("正在构建哈夫曼树......\n");
    root_ = buildHuffmanTree();
    printf("正在产生新编码......\n");
    initCodePoint(root_, 0, string(), 0);
    printf("正在解压缩......\n");
    uncompress(); // 解压缩
    printInfo("解压缩");
    printf("解压成功\n");
}

}

// 统计频率
void HuffmanCodes::statisticalFrequency()
{
	// 打开文件
	FILE *input_fp = nullptr;
	if ((input_fp = fopen(input_filename_.c_str(), "rb")) == nullptr)
	{
		printf("open file %s failed!\n", input_filename_.c_str());
		exit(-1);
	}
	// 统计频率
	while (!feof(input_fp))
	{
		Byte input_byte;
		fread(&input_byte, 1, 1, input_fp);
		if (feof(input_fp))
		{
			break;
		}
		++points_[input_byte].frequency;
		++input_filesize_;
	}

	fclose(input_fp);
}

// 构建 Huffman 树
HuffmanTreeNode *HuffmanCodes::buildHuffmanTree()
{
	// 使用优先队列，自定义比较器(小根堆)
	priority_queue<HuffmanTreeNode *, vector<HuffmanTreeNode *>,
				   CmparatorOfHuffmanTreeNode>
		pq;
	// 初始化叶子节点
	for (int i = 0; i < kCodeNum; ++i)
	{
		pq.push(new HuffmanTreeNode(points_[i].frequency, &points_[i]));
	}
	// 取最小两个节点合并
	while (true)
	{
		HuffmanTreeNode *node1 = pq.top();
		pq.pop();
		HuffmanTreeNode *node2 = pq.top();
		pq.pop();
		HuffmanTreeNode *node3 = new HuffmanTreeNode(
			node1->weight + node2->weight, nullptr, node1, node2);
		// 优先队列为空则成功构建 Huffman 树，返回
		if (pq.empty())
		{
			return node3;
		}
		pq.push(node3);
	}
}

// 获得 Huffman 编码
void HuffmanCodes::initCodePoint(HuffmanTreeNode *node, ULL new_code,
								 string new_code_str, int length)
{
	// 是叶子节点则结束，记录编码
	if (node->point)
	{
		node->point->new_code = new_code;
		node->point->new_code_str = new_code_str;
		node->point->length = length;
		return;
	}
	// 否则向左右分支探索
	new_code <<= 1;
	++length;
	if (node->left)
	{
		initCodePoint(node->left, new_code, new_code_str + "0", length);
	}
	if (node->right)
	{
		initCodePoint(node->right, new_code + 1, new_code_str + "1", length);
	}
}

// 压缩文件
void HuffmanCodes::compress()
{
	FILE *input_fp = nullptr;
	if ((input_fp = fopen(input_filename_.c_str(), "rb")) == nullptr)
	{
		printf("open file %s failed!\n", input_filename_.c_str());
		exit(-1);
	}

	FILE *output_fp = nullptr;
	if ((output_fp = fopen(output_filename_.c_str(), "wb")) == nullptr)
	{
		printf("open file %s failed!\n", output_filename_.c_str());
		exit(-1);
	}

	// 写入压缩文件头信息
	// 识别符
	fwrite(kZipName, kLenOfZipName, 1, output_fp);
	// 文件名
	fwrite(input_filename_.c_str(), kLenOfFileName, 1, output_fp);
	// 文件大小
	fwrite(&input_filesize_, kLenOfFileSize, 1, output_fp);
	// 字符频率，用以构建Huffman树
	/*for (int i = 0; i < kCodeNum; ++i)
	{
		fwrite(&points_[i].frequency, kLenOfCodeFrequency, 1, output_fp);
	}*/

	// 压缩所需临时辅助变量
	Byte input_byte = 0;
	Byte output_byte = 0;
	ULL new_code = 0;
	int length = 0;
	int cnt = 0;
	ULL cur_input_size = 0;
	double cur_rate = 0.0;

	while (!feof(input_fp))
	{
		// FIXME: 每次只读一个字节，可优化
		fread(&input_byte, 1, 1, input_fp);
		// 读到文件尾，结束
		if (feof(input_fp))
		{
			break;
		}

		// 记录速率
		++cur_input_size;
		double rate = static_cast<double>(cur_input_size) /
					  static_cast<double>(input_filesize_) * 100.0;
		if (rate - cur_rate >= 10)
		{
			cur_rate = rate;
			printf("已压缩：%.1f%%\t压缩率: %.2f%%\n", cur_rate,
				   static_cast<double>(output_filesize_) /
					   static_cast<double>(cur_input_size) * 100.0);
		}

		new_code = points_[static_cast<size_t>(input_byte)].new_code;
		length = points_[static_cast<size_t>(input_byte)].length;

		// 复用 output_byte 拼接 new_code 的 bit 流
		// +------------+-----------------+----+------------+
		// | new_code(1) | new_code(2)    | …… | new_code(n)|
		// +------------+-----------------+----+------------+
		// | 10011      | 111 1111 1111   | …… |            |
		// +-----------------+------------+----|------------+
		// |   outByte(1)    | outByte(2) | …… |  ……        |
		// +-----------------+------------+----+------------+
		while (length--)
		{
			// 左移一位，最低位为新的可使用bit, 并将该bit置位 new_code
			// 需要写入的 bit
			output_byte <<= 1;
			output_byte += (new_code >> length) & 1;
			// 如果当前 output_byte 的 8bit 都已用完，写入文件，重新复用
			// FIXME: 1byte复用效率太低，也无缓存机制，可优化
			if (++cnt == 8)
			{
				fwrite(&output_byte, 1, 1, output_fp);
				output_byte = 0;
				cnt = 0;
				++output_filesize_;
			}
		}
	}

	// 最后一个不足8比特填充 0
	if (cnt < 8)
	{
		output_byte <<= 8 - cnt;
		fwrite(&output_byte, 1, 1, output_fp);
		++output_filesize_;
	}

	//  打印压缩信息
	printf("已压缩: %.1f%%\t压缩率: %.2f%%\n", 100.0,
		   (double)output_filesize_ / cur_input_size * 100);

	// 关闭文件
	fclose(input_fp);
	fclose(output_fp);
}

// 搜索节点，辅助于解码
bool HuffmanCodes::findNode(HuffmanTreeNode *&node, Byte input_byte,
							int &pos)
{
	// 叶子节点搜索成功
	if (node->point)
	{
		return true;
	}
	if (pos < 0)
	{
		return false;
	}
	int val = (input_byte >> pos) & 1;
	--pos;
	if (val == 0)
	{
		node = node->left;
		return findNode(node, input_byte, pos);
	}
	else
	{
		node = node->right;
		return findNode(node, input_byte, pos);
	}
}

// 解压缩
void HuffmanCodes::uncompress()
{
	FILE *input_fp = nullptr;
	if ((input_fp = fopen(input_filename_.c_str(), "rb")) == nullptr)
	{
		printf("open file %s failed!\n", input_filename_.c_str());
		exit(-1);
	}
	size_t dotPosition = input_filename_.find_last_of('.');
	if (dotPosition != std::string::npos) {
    output_filename_ = input_filename_.substr(0, dotPosition);
	} else {
    output_filename_ = input_filename_;
	}

	FILE *output_fp = nullptr;
	if ((output_fp = fopen(output_filename_.c_str(), "wb")) == nullptr)
	{
		printf("open file %s failed!\n", output_filename_.c_str());
		exit(-1);
	}

	fseek(input_fp, kLenOfZipHeader, SEEK_SET);
	input_filesize_ = kLenOfZipHeader;

	// 解压缩所需临时变量
	Byte input_byte;
	Byte output_byte;
	HuffmanTreeNode *node = root_;
	ULL cur_output_size = 0;
	int pos;
	double currRate = 0.0;

	while (!feof(input_fp))
	{
		fread(&input_byte, 1, 1, input_fp);
		if (feof(input_fp))
		{
			break;
		}
		++input_filesize_;
		pos = 7;

		while (findNode(node, input_byte, pos))
		{
			output_byte = node->point->old_code;

			fwrite(&output_byte, 1, 1, output_fp);

			// 记录速率
			double rate = (double)(++cur_output_size) / input_filesize_ * 100;
			if (rate - currRate >= 10)
			{
				currRate = rate;
				//	system("cls");
				printf("已解压缩：%.1f%%\t解压缩率: %.2f%%\n", currRate,
					   (double)cur_output_size / input_filesize_ * 100);
			}

			if (cur_output_size == output_filesize_)
			{
				printf("已解压缩：%.1f%%\t解压缩率: %.2f%%\n", 100.0,
					   (double)cur_output_size / input_filesize_ * 100);
				break;
			}

			node = root_;
			output_filesize_ = cur_output_size;
		}
	}
	fclose(input_fp);
	fclose(output_fp);
}

// 打印哈夫曼编码信息
// 打印哈夫曼编码信息
void HuffmanCodes::printHuffmanEncodeInfo(const std::string &outputFileNameSuffix)
{
	std::string outputFileName = input_filename_ + outputFileNameSuffix;
    // 将其写到一个新文件里
    FILE *frequencyP = fopen(outputFileName.c_str(), "w+");

    fprintf(frequencyP, "%-10s %-10s %-100s %-5s %-10s\n", "原码", "频率", "哈夫曼编码",
            "长度", "十进制");

    printf("%-10s %-10s %-100s %-5s %-10s\n", "原码", "频率", "哈夫曼编码",
           "长度", "十进制");

    for (int i = 0; i < kCodeNum; ++i)
    {
        HuffmanCodePoint &code = points_[i];
        printf("%-10d %-10llu %-100s %-5d %-10llu\n", (int)code.old_code,
               code.frequency, code.new_code_str.c_str(), code.length,
               code.new_code);

        fprintf(frequencyP, "%-10d %-10llu %-100s %-5d %-10llu\n", (int)code.old_code,
                code.frequency, code.new_code_str.c_str(), code.length,
                code.new_code);
    }

    fclose(frequencyP);
}


// 打印压缩或解压缩信息
void HuffmanCodes::printInfo(const char *type)
{
	double compress_rate = (double)output_filesize_ / input_filesize_ * 100;
	printf("%s率: %.2f%%\n", type, compress_rate);
	double input_filesize = static_cast<double>(input_filesize_);
	double output_filesize = static_cast<double>(output_filesize_);
	if (input_filesize < 1024)
	{
		printf("输入文件大小：%.2fB, 输出文件大小：%.2fB\n", input_filesize,
			   output_filesize);
		return;
	}

	input_filesize /= 1024;
	output_filesize /= 1024;
	if (input_filesize < 1024)
	{
		printf("输入文件大小：%.2fKB, 输出文件大小：%.2fKB\n", input_filesize,
			   output_filesize);
		return;
	}

	input_filesize /= 1024;
	output_filesize /= 1024;
	if (input_filesize < 1024)
	{
		printf("输入文件大小：%.2fMB, 输出文件大小：%.2fMB\n", input_filesize,
			   output_filesize);
		return;
	}

	input_filesize /= 1024;
	output_filesize /= 1024;
	if (input_filesize < 1024)
	{
		printf("输入文件大小：%.2fGB, 输出文件大小：%.2fGB\n", input_filesize,
			   output_filesize);
		return;
	}
}

// 判断两个文件是否相等
bool HuffmanCodes::equalFile(const string &filename1,
							 const string &filename2)
{
	FILE *fp1 = nullptr;
	if ((fp1 = fopen(filename1.c_str(), "rb")) == nullptr)
	{
		printf("open file %s failed!\n", filename1.c_str());
		exit(-1);
	}

	FILE *fp2 = nullptr;
	if ((fp2 = fopen(filename2.c_str(), "rb")) == nullptr)
	{
		printf("open file %s failed!\n", filename2.c_str());
		exit(-1);
	}

	while (!feof(fp1) && !feof(fp2))
	{
		Byte uch1, uch2;

		fread(&uch1, 1, 1, fp1);
		fread(&uch2, 1, 1, fp2);

		if (uch1 != uch2)
		{
			return false;
		}
	}

	if (!feof(fp1) || !feof(fp2))
	{
		return false;
	}

	return true;
}

