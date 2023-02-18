#include "stl_reader.h"
#include <memory>

class Node
{
    protected:
        float Coord[3];

    public:
        Node() {};
        ~Node() {};
        void SetCoordinate(float x, float y, float z) {
            Coord[0] = x;
            Coord[1] = y;
            Coord[2] = z;
        };
        float* GetCoordinate() {
            return Coord;
        };
        
        Node& operator=(Node& rhs) = delete;
        Node& operator=(Node&& rhs) = delete;
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

    public:
        Face() {};
        ~Face() {};
        void SetNodes(Node& node1, Node& node2, Node& node3) {
            pNodes[0] = std::make_shared<Node>(node1);
            pNodes[1] = std::make_shared<Node>(node2);
            pNodes[2] = std::make_shared<Node>(node3);
        };
        Node* GetNodes() {
            return pNodes->get();
        };

        Face& operator=(Face& rhs) = delete;
        Face& operator=(Face&& rhs) noexcept;
        bool operator==(Face& rhs) {
            for (int i = 0; i < 3; i++)
            {
                if (*this->pNodes[i] != *rhs.pNodes[i]) {
                    return false;
                }
            }
            return true;
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
        void SetNodes(Node& node1, Node& node2, Node& node3, Node& node4);
        Node* GetNodes();
        void SetFaces(Face& face1, Face& face2, Face& face3, Face& face4);
        Face* GetFaces();

        Element& operator=(Element& rhs) = delete;
        Element& operator=(Element&& rhs) noexcept;
        bool operator==(Element& rhs);
        bool operator!=(Element& rhs);
};