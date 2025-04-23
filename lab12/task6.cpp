#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

struct Rectangle {
    float xMin, yMin, xMax, yMax;

    Rectangle(float x1, float y1, float x2 , float y2) :
        xMin(std::min(x1, x2)), yMin(std::min(y1, y2)),
        xMax(std::max(x1, x2)), yMax(std::max(y1, y2)) {}

    float area() const {
        return (xMax - xMin) * (yMax - yMin);
    }

    bool intersects(const Rectangle& other) const {
        return !(xMax < other.xMin || xMin > other.xMax ||
                 yMax < other.yMin || yMin > other.yMax);
    }

    void expandToInclude(const Rectangle& other) {
        xMin = std::min(xMin, other.xMin);
        yMin = std::min(yMin, other.yMin);
        xMax = std::max(xMax, other.xMax);
        yMax = std::max(yMax, other.yMax);
    }
};


struct Node {
    bool isLeaf;
    std::vector<Rectangle> entries;
    std::vector<Node*> children;

    Node(bool isLeaf = true) : isLeaf(isLeaf) {}

    ~Node() {
        for (auto child : children) {
            delete child;
        }
    }

    Rectangle getMBR() const {
        if (entries.empty()) return Rectangle(0, 0, 0, 0);
        Rectangle mbr = entries[0];
        for (const auto& r : entries) {
            mbr.expandToInclude(r);
        }
        return mbr;
    }

    void removeEntry(int index) {
        entries.erase(entries.begin() + index);
        if (!isLeaf) {
            delete children[index];
            children.erase(children.begin() + index);
        }
    }
};

class RTree {
private:
    Node* root;
    int maxEntries;
    int minEntries;
    
    Node* chooseLeaf(Node* node, const Rectangle& rect, std::vector<Node*>& path) {
        path.push_back(node);
        if (node->isLeaf) {
            return node;
        }

        float minEnlargement = std::numeric_limits<float>::max();
        float minArea = std::numeric_limits<float>::max();
        int bestIndex = 0;

        for (int i = 0; i < node->entries.size(); ++i) {
            Rectangle temp = node->entries[i];
            float areaBefore = temp.area();
            temp.expandToInclude(rect);
            float enlargement = temp.area() - areaBefore;

            if (enlargement < minEnlargement ||
                (enlargement == minEnlargement && temp.area() < minArea)) {
                minEnlargement = enlargement;
                minArea = temp.area();
                bestIndex = i;
            }
        }

        return chooseLeaf(node->children[bestIndex], rect, path);
    }

    
    void split(Node* node, Node*& newNode) {
        newNode = new Node(node->isLeaf);
        int seed1 = 0, seed2 = 0;
        float maxWaste = -std::numeric_limits<float>::max();

        for (int i = 0; i < node->entries.size(); ++i) {
            for (int j = i + 1; j < node->entries.size(); ++j) {
                Rectangle combined = node->entries[i];
                combined.expandToInclude(node->entries[j]);
                float waste = combined.area() - node->entries[i].area() - node->entries[j].area();
                if (waste > maxWaste) {
                    maxWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        
        std::vector<bool> assigned(node->entries.size(), false);
        assigned[seed1] = true;
        assigned[seed2] = true;

        Rectangle group1 = node->entries[seed1];
        Rectangle group2 = node->entries[seed2];
        std::vector<int> group1Entries = {seed1};
        std::vector<int> group2Entries = {seed2};

        while (std::count(assigned.begin(), assigned.end(), false) > 0) {
            float maxDiff = -1;
            int nextEntry = -1;
            bool assignToGroup1;

            for (int i = 0; i < node->entries.size(); ++i) {
                if (!assigned[i]) {
                    Rectangle temp1 = group1;
                    temp1.expandToInclude(node->entries[i]);
                    float enlargement1 = temp1.area() - group1.area();

                    Rectangle temp2 = group2;
                    temp2.expandToInclude(node->entries[i]);
                    float enlargement2 = temp2.area() - group2.area();

                    float diff = std::abs(enlargement1 - enlargement2);
                    if (diff > maxDiff) {
                        maxDiff = diff;
                        nextEntry = i;
                        assignToGroup1 = (enlargement1 < enlargement2);
                    }
                }
            }

            assigned[nextEntry] = true;
            if (assignToGroup1 || (group1Entries.size() + assigned.size() - std::count(assigned.begin(), assigned.end(), false) < minEntries)) {
                group1.expandToInclude(node->entries[nextEntry]);
                group1Entries.push_back(nextEntry);
            } else {
                group2.expandToInclude(node->entries[nextEntry]);
                group2Entries.push_back(nextEntry);
            }
        }
        
        Node* new1 = new Node(node->isLeaf);
        Node* new2 = new Node(node->isLeaf);

        for (int i : group1Entries) {
            new1->entries.push_back(node->entries[i]);
            if (!node->isLeaf) new1->children.push_back(node->children[i]);
        }

        for (int i : group2Entries) {
            new2->entries.push_back(node->entries[i]);
            if (!node->isLeaf) new2->children.push_back(node->children[i]);
        }
        
        *node = *new1;
        *newNode = *new2;
        delete new1;
    }
    
    void condenseTree(std::vector<Node*>& path) {
        for (int i = path.size() - 1; i >= 0; --i) {
            Node* node = path[i];
            if (node->entries.size() < minEntries) {
                if (i == 0) { 
                    if (!node->isLeaf && node->children.size() == 1) {
                        Node* newRoot = node->children[0];
                        node->children.clear();
                        delete root;
                        root = newRoot;
                    }
                    break;
                } else {
                    Node* parent = path[i - 1];
                    int index = -1;
                    for (int j = 0; j < parent->children.size(); ++j) {
                        if (parent->children[j] == node) {
                            index = j;
                            break;
                        }
                    }
                    
                    for (auto& entry : node->entries) {
                        parent->children[index]->entries.push_back(entry);
                    }
                    for (auto& child : node->children) {
                        parent->children[index]->children.push_back(child);
                    }
                    node->entries.clear();
                    node->children.clear();

                    parent->removeEntry(index);
                }
            } else {
                if (i > 0) {
                    Node* parent = path[i - 1];
                    for (int j = 0; j < parent->children.size(); ++j) {
                        if (parent->children[j] == node) {
                            parent->entries[j] = node->getMBR();
                            break;
                        }
                    }
                }
            }
        }
    }

public:
    RTree(int maxE = 4, int minE = 2) : maxEntries(maxE), minEntries(minE) {
        root = new Node(true);
    }

    ~RTree() {
        delete root;
    }

    void insert(const Rectangle& rect) {
        std::vector<Node*> path;
        Node* leaf = chooseLeaf(root, rect, path);

        leaf->entries.push_back(rect);
        if (leaf->entries.size() > maxEntries) {
            Node* newLeaf = nullptr;
            split(leaf, newLeaf);
            path.pop_back();
            adjustTree(leaf, newLeaf, path);
        } else {
            adjustTree(leaf, nullptr, path);
        }
    }

    void adjustTree(Node* node, Node* newNode, std::vector<Node*>& path) {
        if (path.empty()) {
            if (newNode) {
                Node* newRoot = new Node(false);
                newRoot->entries = {node->getMBR(), newNode->getMBR()};
                newRoot->children = {node, newNode};
                root = newRoot;
            }
            return;
        }

        Node* parent = path.back();
        path.pop_back();

        for (int i = 0; i < parent->children.size(); ++i) {
            if (parent->children[i] == node) {
                parent->entries[i] = node->getMBR();
                break;
            }
        }

        if (newNode) {
            parent->entries.push_back(newNode->getMBR());
            parent->children.push_back(newNode);

            if (parent->entries.size() > maxEntries) {
                Node* splitNode = nullptr;
                split(parent, splitNode);
                adjustTree(parent, splitNode, path);
            }
        }

        adjustTree(parent, nullptr, path);
    }

    bool remove(const Rectangle& rect) {
        std::vector<Node*> path;
        if (removeHelper(root, rect, path)) {
            condenseTree(path);
            return true;
        }
        return false;
    }

    bool removeHelper(Node* node, const Rectangle& rect, std::vector<Node*>& path) {
        path.push_back(node);
        if (node->isLeaf) {
            for (int i = 0; i < node->entries.size(); ++i) {
                if (std::abs(node->entries[i].xMin - rect.xMin) < 0.1 &&
                    std::abs(node->entries[i].xMax - rect.xMax) < 0.1 &&
                    std::abs(node->entries[i].yMin - rect.yMin) < 0.1 &&
                    std::abs(node->entries[i].yMax - rect.yMax) < 0.1) {
                    node->entries.erase(node->entries.begin() + i);
                    return true;
                }
            }
            return false;
        }

        for (int i = 0; i < node->entries.size(); ++i) {
            if (node->entries[i].intersects(rect)) {
                if (removeHelper(node->children[i], rect, path)) {
                    if (node->children[i]->entries.size() < minEntries) {
                        path.pop_back();
                        condenseTree(path);
                    } else {
                        node->entries[i] = node->children[i]->getMBR();
                    }
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<Rectangle> search(const Rectangle& area) {
        std::vector<Rectangle> results;
        searchHelper(root, area, results);
        return results;
    }

    void searchHelper(Node* node, const Rectangle& area, std::vector<Rectangle>& results) {
        for (int i = 0; i < node->entries.size(); ++i) {
            if (node->entries[i].intersects(area)) {
                if (node->isLeaf) {
                    results.push_back(node->entries[i]);
                } else {
                    searchHelper(node->children[i], area, results);
                }
            }
        }
    }
};

int main() {

   // Пример использования
    
    RTree tree(4, 2);

    tree.insert(Rectangle(1, 1, 3, 3));
    tree.insert(Rectangle(2, 2, 4, 4));
    tree.insert(Rectangle(5, 5, 6, 6));
    tree.insert(Rectangle(7, 7, 9, 9));

    auto  rects = tree.search(Rectangle(2, 2, 5, 5));
    std::cout << "Has intersection with " << rects.size() << " rectangles:\n";
    for (auto& r : rects) {
        std::cout << " (" << r.xMin << "," << r.yMin << ") ("
                  << r.xMax << "," << r.yMax << ")\n";
    }

    tree.remove(Rectangle(5, 5, 6, 6));


    rects = tree.search(Rectangle(2, 2, 5, 5));
    std::cout << "Has intersection with " << rects.size() << " rectangles:\n";
    for (auto& r : rects) {
        std::cout << " (" << r.xMin << "," << r.yMin << ")("
                  << r.xMax << "," << r.yMax << ")\n";
    }
}

/*
Вывод

Has intersection with 3 rectangles:
(1,1) (3,3)
(2,2) (4,4)
(5,5) (6,6)
Has intersection with 2 rectangles:
(1,1)(3,3)
(2,2)(4,4)

 */