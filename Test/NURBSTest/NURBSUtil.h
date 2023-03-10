#include <memory>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>
#include <unordered_map>

#include "3Dmath.h"

using namespace Math;

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