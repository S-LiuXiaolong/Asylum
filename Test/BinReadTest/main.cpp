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

void test_read_cpts(std::vector<float>& buffer)
{
    float temp;
    std::string strFile = "../../../Asset/controlPts.bin";
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

void test_read_eles(std::vector<uint32_t>& buffer)
{
    uint32_t temp;
	std::string strFile = "../../../Asset/element.bin";
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

	while (infile.read((char*)&temp, sizeof(uint32_t)))
	{
		int readedBytes = infile.gcount(); //看刚才读了多少字节
		// printf("%f\n", temp);
		buffer.push_back(temp);
	}

}

int main()
{
    test_backup();

    std::vector<float> buffer_cpts;
    test_read_cpts(buffer_cpts);

    for (int i = 0; i < buffer_cpts.size() / 3; i++)
    {
        printf("%f %f %f\n", buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2]);
    }

    std::vector<uint32_t> buffer_eles;
    test_read_eles(buffer_eles);
	for (int i = 0; i < buffer_eles.size() / 27; i++)
	{
        for (int j = 0; j < 27; j++)
        {
            printf("%u ", buffer_eles[i * 27 + j]);
        }
        printf("\n");
	}


    return 0;
}