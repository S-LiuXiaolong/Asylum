#include <memory>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>

#include "3Dmath.h"
#include "xxhash.h"

using namespace Math;

#define isNum(c) (isdigit(c)?c-48:(c=='E'?10:(c=='.'?11:(c=='-'?12:(c=='+'?13:-1)))))

double str2num(std::string s)
{	//字符串转数字，包括整数、小数和科学记数法 
	int i, j, k, negative = 0;
	double n = 0;
	std::string s1, s2;

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

class Node
{
protected:
	float Coord[3];

public:
	Node() {};
	~Node() {};
	//         Node(Node& node) {
	//             std::cout << "copy construction" << std::endl;
	//         }
	//         Node(Node&& node) {
	//             std::cout << "move construction" << std::endl;
	//         }
	void SetCoordinate(float x, float y, float z) {
		Coord[0] = x;
		Coord[1] = y;
		Coord[2] = z;
	};
	float* GetCoordinate() {
		return Coord;
	};

	Node& operator=(Node& rhs) = delete;
	Node& operator=(const Node& rhs) = delete;

	bool operator==(Node& rhs) {
		for (int i = 0; i < 3; i++)
		{
			if (this->Coord[i] != rhs.Coord[i]) {
				return false;
			}
		}
		return true;
	};

	bool operator!=(Node& rhs) {
		for (int i = 0; i < 3; i++)
		{
			if (this->Coord[i] != rhs.Coord[i]) {
				return true;
			}
		}
		return false;
	};
};

// TODO：要注意的是，OpenGL默认面向观察者的面是同向的（假定为逆时针），面剔除会剔除掉所有顺时针面
// 这个问题将在Element类中解决：所有面的法向量均向外即可
class Face
{
protected:
	std::shared_ptr<Node> pNodes[3];
	int nodeIndex[3];
	Vector3 normal;

public:
	Face() {};
	~Face() {};

	void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3, int index[3]) {
		for (int i = 0; i < 3; i++) {
			nodeIndex[i] = index[i];
		}
		SetNodes(node1, node2, node3);
	};

	void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3) {
		pNodes[0] = node1;
		pNodes[1] = node2;
		pNodes[2] = node3;
	};

	void SetNormal(Vector3& vec) {
		normal = vec;
	}

	// 		void SetNodes(Node& node1, Node& node2, Node& node3) {
	// 			pNodes[0] = std::make_shared<Node>(node1);
	// 			pNodes[1] = std::make_shared<Node>(node2);
	// 			pNodes[2] = std::make_shared<Node>(node3);
	// 		};

	std::shared_ptr<Node>* GetNodes() {
		return pNodes;
	};

	Vector3 GetNormal() {
		return normal;
	}

	int* GetIndex()
	{
		return nodeIndex;
	}

	Face& operator=(Face& rhs) = delete;

	// TODO: 如果两个面的点相同，但是排列顺序不同？(solved)
	bool operator==(Face& rhs) {
		bool flag[3] = { false };
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (*this->pNodes[i] == *rhs.pNodes[j]) {
					flag[i] = true;
				}
			}
		}

		return (flag[0] && flag[1] && flag[2]);
	};

};

class Element
{
protected:
	int nodeIndex[4];
	std::shared_ptr<Node> pNodes[4];
	std::shared_ptr<Face> pFaces[4];

private:
	// Let the normal vectors of all faces point outside the tetrahedron
	void GenerateFaces(Vector3 eleGravityCenter)
	{
		int count = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = i + 1; j < 4; j++) {
				for (int k = j + 1; k < 4; k++) {
					Face faceGen;

					Vector3 vecGravity;

					Vector3 node1Vec(pNodes[i].get()->GetCoordinate()[0], pNodes[i].get()->GetCoordinate()[1], pNodes[i].get()->GetCoordinate()[2]);
					Vector3 node2Vec(pNodes[j].get()->GetCoordinate()[0], pNodes[j].get()->GetCoordinate()[1], pNodes[j].get()->GetCoordinate()[2]);
					Vector3 node3Vec(pNodes[k].get()->GetCoordinate()[0], pNodes[k].get()->GetCoordinate()[1], pNodes[k].get()->GetCoordinate()[2]);

					float x = (node1Vec.x + node2Vec.x + node3Vec.x) / 3;
					float y = (node1Vec.y + node2Vec.y + node3Vec.y) / 3;
					float z = (node1Vec.z + node2Vec.z + node3Vec.z) / 3;
					Vector3 faceGravityCenter(x, y, z);

					Vec3Subtract(vecGravity, faceGravityCenter, eleGravityCenter);

					// vec1 = node2 - node1; vec2 = node3 - node1
					Vector3 vec1;
					Vector3 vec2;
					Vec3Subtract(vec1, node2Vec, node1Vec);
					Vec3Subtract(vec2, node3Vec, node1Vec);

					Vector3 crossProduct;
					Vec3Cross(crossProduct, vec1, vec2);


					if (Vec3Dot(vecGravity, crossProduct) > 0) {
						int index[3] = { nodeIndex[i], nodeIndex[j], nodeIndex[k] };
						faceGen.SetNodes(pNodes[i], pNodes[j], pNodes[k], index);
						faceGen.SetNormal(crossProduct);
					}
					else {
						int index[3] = { nodeIndex[i], nodeIndex[k], nodeIndex[j] };
						faceGen.SetNodes(pNodes[i], pNodes[k], pNodes[j], index);
						Vector3 reverse;
						Vec3Scale(reverse, crossProduct, -1);
						faceGen.SetNormal(reverse);
					}

					pFaces[count] = std::make_shared<Face>(faceGen);
					count++;
				}
			}
		}

	}

public:
	Element() {};
	~Element() {};
	void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3, std::shared_ptr<Node> node4) {
		pNodes[0] = node1;
		pNodes[1] = node2;
		pNodes[2] = node3;
		pNodes[3] = node4;

		float x = (node1.get()->GetCoordinate()[0] + node2.get()->GetCoordinate()[0] + node3.get()->GetCoordinate()[0] + node4.get()->GetCoordinate()[0]) / 4;
		float y = (node1.get()->GetCoordinate()[1] + node2.get()->GetCoordinate()[1] + node3.get()->GetCoordinate()[1] + node4.get()->GetCoordinate()[1]) / 4;
		float z = (node1.get()->GetCoordinate()[2] + node2.get()->GetCoordinate()[2] + node3.get()->GetCoordinate()[2] + node4.get()->GetCoordinate()[2]) / 4;
		Vector3 eleGravityCenter(x, y, z);

		GenerateFaces(eleGravityCenter);
	}

	void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3, std::shared_ptr<Node> node4, int index[4]) {
		for (int i = 0; i < 4; i++) {
			nodeIndex[i] = index[i];
		}
		SetNodes(node1, node2, node3, node4);
	}

	std::shared_ptr<Node>* GetNodes() {
		return pNodes;
	};
	// TODO: inp文件的element由四个点来定义，所以面片需要通过四个点的排列组合来得到
//        void SetFaces(std::shared_ptr<Face> face1, std::shared_ptr<Face> face2, std::shared_ptr<Face> face3, std::shared_ptr<Face> face4) {
//            pFaces[0] = face1;
//            pFaces[1] = face2;
//            pFaces[2] = face3;
//            pFaces[3] = face4;
//        }
	std::shared_ptr<Face>* GetFaces() {
		return pFaces;
	}

	Element& operator=(Element& rhs) = delete;

	// TODO: rewrite here (but seemingly useless)
	bool operator==(Element& rhs) {
		for (int i = 0; i < 4; i++)
		{
			if (*this->pNodes[i] != *rhs.pNodes[i]) {
				return false;
			}
		}
		return true;
	};
};

class INPMesh
{
	struct isExistStruct
	{
		size_t hashvalue;
		int flag = 0;
		std::shared_ptr<Face> face;
	};

protected:
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<Element>> elements;

	Vector3 bbmin{ FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 bbmax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	std::vector<int> indexbuffer;

private:
	void PushNode(std::vector<std::string>& numsStr)
	{
		std::shared_ptr<Node> node(new Node);
		std::vector<float> numsFloat;
		for (auto& i = numsStr.begin(); i != numsStr.end(); i++) {
			numsFloat.emplace_back(float(str2num(*i)));
		}

		node->SetCoordinate(numsFloat[1], numsFloat[2], numsFloat[3]);
		nodes.push_back(node);
	}

	// TODO: ignore indices of inp. element cause of convenience
	void PushElement(std::vector<std::string>& numsStr)
	{
		std::shared_ptr<Element> ele(new Element);
		std::vector<int> numsInt;
		for (auto& i = numsStr.begin(); i != numsStr.end(); i++) {
			numsInt.emplace_back(int(str2num(*i)));
		}

		// Only for C3D4 type element
		// Attention: index here start from zero and used for OpenGL indexbuffer
		// start from numsInt[1] to prevent indices of .inp file own
		int index[4] = { numsInt[1] - 1 , numsInt[2] - 1 , numsInt[3] - 1 , numsInt[4] - 1 };
		ele->SetNodes(nodes[index[0]], nodes[index[1]], nodes[index[2]], nodes[index[3]], index);
		elements.push_back(ele);
	}

	void SetIndexBuffer()
	{
		for (auto& ele : elements) {
			for (int i = 0; i < 4; i++) {
				Face face = *(ele->GetFaces()[i]);
				for (int j = 0; j < 3; j++) {
					indexbuffer.push_back(face.GetIndex()[j]);
				}
			}
		}
	}

public:
	std::vector<std::shared_ptr<Node>> GetNodes() { return nodes; }
	std::vector<std::shared_ptr<Element>> GetElements() { return elements; }
	std::vector<int> GetIndexBuffer() { return indexbuffer; }
	Vector3 GetAABBmin() { return bbmin; }
	Vector3 GetAABBmax() { return bbmax; }

	// std::vector<std::shared_ptr<Node>> GetOuterSurface()
	std::vector<std::shared_ptr<Face>> GetOuterSurface()
	{
		std::vector<std::shared_ptr<Face>> OuterFaces;
		std::vector<isExistStruct> surfaceHash[10000];

		std::cout << "loop 1 begin" << std::endl;
		for (auto& ele : elements)
		{
			for (int i = 0; i < 4; i++)
			{
				std::shared_ptr<Face> face = ele->GetFaces()[i];
				int* index = face->GetIndex();
				std::sort(index, index + 3);
				// XXH64_hash_t here on 64-bit machine is uint64_t
				XXH64_hash_t hash = XXH64(index, 12, 0);
				int division = hash / 10000000000000000;
				size_t tail = hash % 10000000000000000;

				bool isExist = false;
				for (auto& iter : surfaceHash[division])
				{
					if (tail == iter.hashvalue) {
						isExist = true;
						iter.flag = -1;
						break;
					}
				}

				if (!isExist) {
					surfaceHash[division].push_back({ tail, 1, face });
				}
			}
		}

		std::cout << "loop 2 begin" << std::endl;
		for (int i = 0; i < 10000; i++)
		{
			for (auto& iter : surfaceHash[i])
			{
				if (iter.flag == 1)
				{
					OuterFaces.push_back(iter.face);
				}
			}
		}

		return OuterFaces;
	}

	// TODO
	void LoadFromFile(std::string inp_file_path)
	{
		bool isINP = false;
		size_t extpos = inp_file_path.rfind('.', inp_file_path.length());
		if (extpos != std::string::npos) {
			isINP = (inp_file_path.substr(extpos + 1, inp_file_path.length() - extpos) == "inp");
		}

		if (!isINP) {
			std::cout << "Not a .inp file!" << std::endl;
			return;
		}

		std::ifstream infile;
		infile.open(inp_file_path);

		if (!infile) {
			std::cout << "Open File Fail!" << std::endl;
			return;
		}
		// read .inp text content to a string
		std::string readStr((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

		for (int i = 0; i < readStr.size(); i++) {
			// Change all strings into uppercase
			readStr[i] = toupper(readStr[i]);
		}

		// find pos of "*NODE" and "*ELEMENT"
		size_t nodestart = readStr.find('\n', readStr.find("*NODE"));
		size_t elestart = readStr.find('\n', readStr.find("*ELEMENT"));

		size_t currentpos = 0;
		// read all node information
		{
			currentpos = nodestart;
			while (currentpos < elestart)
			{
				if (readStr[currentpos + 1] == ' ') {
					std::vector<std::string> nums;

					size_t lineendpos = readStr.find('\n', currentpos + 1);
					while (currentpos < lineendpos) {
						size_t numendpos = readStr.find(',', currentpos + 1);
						std::string temp = readStr.substr(currentpos + 1, numendpos - currentpos);
						size_t numstartpos = temp.rfind(' ') + currentpos + 1;
						if (numendpos < lineendpos) {
							std::string numStr = readStr.substr(numstartpos + 1, numendpos - numstartpos - 1);
							currentpos = numendpos;

							nums.push_back(numStr);
						}
						else {
							numendpos = lineendpos;
							std::string temp = readStr.substr(currentpos + 1, numendpos - currentpos);
							size_t numstartpos = temp.rfind(' ') + currentpos + 1;
							std::string numStr = readStr.substr(numstartpos + 1, numendpos - numstartpos - 1);
							currentpos = lineendpos;

							nums.push_back(numStr);

							PushNode(nums);
						}
					}
				}
				// 					else if (readStr[currentpos + 1] != ' ') {
				// 						std::cout << "Now support only .inp file with at least one space ahead of every line.";
				// 					}
				else {
					currentpos = elestart;
					std::cout << "Node content read finish" << std::endl;
				}
			}
		}

		// read all element information
		{
			while (1)
			{
				if (readStr[currentpos + 1] == ' ') {
					std::vector<float> numsFloat;
					std::vector<std::string> nums;

					size_t lineendpos = readStr.find('\n', currentpos + 1);
					while (currentpos < lineendpos) {
						size_t numendpos = readStr.find(',', currentpos + 1);
						std::string temp = readStr.substr(currentpos + 1, numendpos - currentpos);
						size_t numstartpos = temp.rfind(' ') + currentpos + 1;
						if (numendpos < lineendpos) {
							std::string numStr = readStr.substr(numstartpos + 1, numendpos - numstartpos - 1);
							currentpos = numendpos;

							nums.push_back(numStr);
						}
						else {
							numendpos = lineendpos;
							std::string temp = readStr.substr(currentpos + 1, numendpos - currentpos);
							size_t numstartpos = temp.rfind(' ') + currentpos + 1;
							std::string numStr = readStr.substr(numstartpos + 1, numendpos - numstartpos - 1);
							currentpos = lineendpos;

							nums.push_back(numStr);

							PushElement(nums);
						}
					}
				}
				else {
					std::cout << "Element content read finish" << std::endl;
					break;
				}
			}
		}

		// SetIndexBuffer();

		for (auto& node : nodes)
		{
			for (int i = 0; i < 3; i++)
			{
				if ((node.get()->GetCoordinate())[i] > bbmax[i]) {
					bbmax[i] = (node.get()->GetCoordinate())[i];
				}
				if ((node.get()->GetCoordinate())[i] < bbmin[i]) {
					bbmin[i] = (node.get()->GetCoordinate())[i];
				}
			}
		}
	}

};