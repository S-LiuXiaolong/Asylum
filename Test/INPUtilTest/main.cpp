#include "inpUtil.h"
#include <iostream>

void Test()
{
    std::shared_ptr<Node> node1(new Node());
    std::shared_ptr<Node> node2(new Node());
    std::shared_ptr<Node> node3(new Node());
    std::shared_ptr<Node> node4(new Node());
    std::shared_ptr<Node> node5(new Node());
    node1->SetCoordinate(1.0f, 1.0f, 0.0f);
    node2->SetCoordinate(1.0f, 0.0f, 0.0f);
    node3->SetCoordinate(1.5f, 0.5f, 0.707f);
    node4->SetCoordinate(0.5f, 0.5f, 0.707f);
    node5->SetCoordinate(1.0f, 2.0f, 2.0f);


    if (*node2.get() == *node3.get()) {
        std::cout << "node2 = node3 pass" << std::endl;
    }
    else {
        std::cout << "node2 = node3 not pass" << std::endl;
    }

    std::shared_ptr<Face> face1(new Face());
    std::shared_ptr<Face> face2(new Face());
    std::shared_ptr<Face> face3(new Face());
    std::shared_ptr<Face> face4(new Face());
    std::shared_ptr<Face> face5(new Face());
    face1->SetNodes(node1, node2, node3);
    face2->SetNodes(node1, node2, node3);
    face3->SetNodes(node1, node2, node5);
    face4->SetNodes(node2, node3, node4);
    face5->SetNodes(node2, node3, node5);


	if (*face1.get() == *face2.get()) {
		std::cout << "face1 = face2 pass" << std::endl;
	}

	std::shared_ptr<Element> ele1(new Element());
	std::shared_ptr<Element> ele2(new Element());
	ele1->SetNodes(node1, node2, node3, node4);
	ele2->SetNodes(node1, node2, node3, node5);

	if (*ele1.get() == *ele2.get()) {
		std::cout << "ele1 = ele2 pass" << std::endl;
	}

    INPMesh mesh;
    // TODO: add AssetLoader class to clean the path
    mesh.LoadFromFile("../../../Asset/Mesh/INP/t13_simple.inp");

}

int main()
{
    Test();

    return 0;
}