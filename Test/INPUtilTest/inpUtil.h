#include <memory>
#include <iostream>

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
class Face
{
    protected:
        std::shared_ptr<Node> pNodes[3];

		void AntiClockWise()
		{
            Node 
		}

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
        bool operator!=(Face& rhs) {
            for (int i = 0; i < 3; i++)
            {
                if (*this->pNodes[i] != *rhs.pNodes[i]) {
                    return true;
                }
            }
            return false;
        };
};

class Element
{
   protected:
       std::shared_ptr<Node> pNodes[4];
       std::shared_ptr<Face> pFaces[4];

   public:
       Element() {};
       ~Element() {};
       void SetNodes(std::shared_ptr<Node> node1, std::shared_ptr<Node> node2, std::shared_ptr<Node> node3,std::shared_ptr<Node> node4) {
		   pNodes[0] = node1;
		   pNodes[1] = node2;
		   pNodes[2] = node3;
		   pNodes[3] = node4;
       }
       Node* GetNodes() {
           return pNodes->get();
       };
       // TODO: inp文件的element由四个点来定义，所以面片需要通过四个点的排列组合来得到
       void SetFaces(std::shared_ptr<Face> face1, std::shared_ptr<Face> face2, std::shared_ptr<Face> face3, std::shared_ptr<Face> face4) {
           pFaces[0] = face1;
           pFaces[1] = face2;
           pFaces[2] = face3;
           pFaces[3] = face4;
       }
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
	   bool operator!=(Element& rhs) {
		   for (int i = 0; i < 4; i++)
		   {
			   if (*this->pNodes[i] != *rhs.pNodes[i]) {
				   return true;
			   }
		   }
		   return false;
	   };
};