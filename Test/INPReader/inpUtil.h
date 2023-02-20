#include <memory>
#include <iostream>

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

// TODO：要注意的是，OpenGL默认面向观察者的面是同向的（假定为逆时针），面剔除会剔除掉所有顺时针面
// 这个问题将在Element类中解决：所有面的法向量均向外即可
class Face
{
    protected:
        std::shared_ptr<Node> pNodes[3];

    public:
        Face() {};
        ~Face() {};

        void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3) {
            pNodes[0] = node1;
            pNodes[1] = node2;
            pNodes[2] = node3;
        };

        // 		void SetNodes(Node& node1, Node& node2, Node& node3) {
        // 			pNodes[0] = std::make_shared<Node>(node1);
        // 			pNodes[1] = std::make_shared<Node>(node2);
        // 			pNodes[2] = std::make_shared<Node>(node3);
        // 		};

        Node* GetNodes() {
            return pNodes->get();
        };

        Face& operator=(Face& rhs) = delete;

        // TODO: 如果两个面的点相同，但是排列顺序不同？(solved)
        bool operator==(Face& rhs) {
            bool flag[3] = {false};
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
       std::shared_ptr<Node> pNodes[4];
       std::shared_ptr<Face> pFaces[4];

   private:
       // Let the normal vectors of all faces point outside the tetrahedron
       Face GenerateOneFace(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3, Vector3 eleGravityCenter)
       {
           Face faceGen;

           Vector3 vecGravity;

           Vector3 node1Vec(node1.get()->GetCoordinate()[0], node1.get()->GetCoordinate()[1], node1.get()->GetCoordinate()[2]);
           Vector3 node2Vec(node2.get()->GetCoordinate()[0], node2.get()->GetCoordinate()[1], node2.get()->GetCoordinate()[2]);
           Vector3 node3Vec(node3.get()->GetCoordinate()[0], node3.get()->GetCoordinate()[1], node3.get()->GetCoordinate()[2]);

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
               faceGen.SetNodes(node1, node2, node3);
           }
           else {
               faceGen.SetNodes(node1, node3, node2);
           }

           return faceGen;
       }
       void GenerateFaces(Vector3 eleGravityCenter)
       {
           int count = 0;
           for (int i = 0; i < 4; i++) {
               for (int j = i + 1; j < 4; j++) {
                   for (int k = j + 1; k < 4; k++) {
                       pFaces[count] = std::make_shared<Face>(
                           GenerateOneFace(pNodes[i], pNodes[j], pNodes[k], eleGravityCenter));
                       count++;
                   }
               }
           }

       }

   public:
       Element() {};
       ~Element() {};
       void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3,std::shared_ptr<Node> node4) {
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
       Node* GetNodes() {
           return pNodes->get();
       };
       // TODO: inp文件的element由四个点来定义，所以面片需要通过四个点的排列组合来得到
//        void SetFaces(std::shared_ptr<Face> face1, std::shared_ptr<Face> face2, std::shared_ptr<Face> face3, std::shared_ptr<Face> face4) {
//            pFaces[0] = face1;
//            pFaces[1] = face2;
//            pFaces[2] = face3;
//            pFaces[3] = face4;
//        }
       Face* GetFaces() {
           return pFaces->get();
       }

       Element& operator=(Element& rhs) = delete;

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