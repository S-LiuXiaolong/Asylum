#include <iostream>
#include <string>
#include <vector>
#include <fstream>

bool ReadFile(std::string& strFile, std::vector<char>& buffer)
{
    std::ifstream infile(strFile.c_str(), std::ifstream::binary);
    if (!infile.is_open())
    {
        printf("Read File:%s Error ... \n", strFile.c_str());
        return false;
    }

    // 获取文件大小
    infile.seekg(0, std::ifstream::end);
    long size = infile.tellg();
    infile.seekg(0);

    buffer.resize(size);
    printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

    // read content of infile
    infile.read(&buffer[0], size);
    infile.close();
    return true;
}

bool WriteFile(std::string& strFile, std::vector<char>& buffer)
{
    std::ofstream outfile(strFile.c_str(), std::ifstream::binary);
    if (!outfile.is_open())
    {
        printf("Write File:%s Error ... \n", strFile.c_str());
        return false;
    }
    outfile.write(&buffer[0], buffer.size());
    outfile.close();
    return true;
}

void test_backup()
{
    std::string oldFile = "../../../Asset/test.bin";
    std::vector<char> buffer;
    if (ReadFile(oldFile, buffer))
    {
        std::string newFile("../../../Asset/test_new.bin");
        if (WriteFile(newFile, buffer))
        {
            printf("File backup %s --> %s successfully ... \n", oldFile.c_str(), newFile.c_str());
        }
    }
}

void test_read_float(std::vector<float>& buffer)
{
    float temp;
    std::string strFile = "../../../Asset/test.bin";
    std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
        return;
	}

	// 获取文件大小
	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
	infile.seekg(0);

	printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

    while (infile.read((char*)&temp, sizeof(float)))
    {
        int readedBytes = infile.gcount(); //看刚才读了多少字节
        // printf("%f\n", temp);
        buffer.push_back(temp);
    }
}

int main()
{
    test_backup();

    std::vector<float> buffer;
    test_read_float(buffer);

    for (int i = 0; i < buffer.size() / 3; i++)
    {
        printf("%f %f %f\n", buffer[i * 3], buffer[i * 3 + 1], buffer[i * 3 + 2]);
    }
    return 0;
}