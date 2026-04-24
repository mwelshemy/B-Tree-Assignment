#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

int totalSplits = 0;

class BTreeNode {
public:
    int *keys;
    int t;
    BTreeNode **children;
    int n;
    bool leaf;

    BTreeNode(int _t, bool _leaf) {
        t = _t;
        leaf = _leaf;
        keys = new int[2 * t - 1];
        children = new BTreeNode *[2 * t];
        n = 0;
    }

    void insertNonFull(int k);
    void splitChild(int i, BTreeNode *y);
    
    bool search(int k) {
        int i = 0;
        while (i < n && k > keys[i]) i++;
        if (i < n && keys[i] == k) return true; 
        if (leaf) return false; 
        return children[i]->search(k); 
    }

    void serialize(ofstream& out) {
        out << leaf << " " << n << " ";
        for (int i = 0; i < n; i++) out << keys[i] << " ";
        out << endl;
        if (!leaf) {
            for (int i = 0; i <= n; i++) children[i]->serialize(out);
        }
    }

    int getHeight() {
        if (leaf) return 0;
        return 1 + children[0]->getHeight();
    }
};

void BTreeNode::splitChild(int i, BTreeNode *y) {
    totalSplits++;
    BTreeNode *z = new BTreeNode(y->t, y->leaf);
    z->n = t - 1;
    for (int j = 0; j < t - 1; j++) z->keys[j] = y->keys[j + t];
    if (!y->leaf) {
        for (int j = 0; j < t; j++) z->children[j] = y->children[j + t];
    }
    y->n = t - 1;
    for (int j = n; j >= i + 1; j--) children[j + 1] = children[j];
    children[i + 1] = z;
    for (int j = n - 1; j >= i; j--) keys[j + 1] = keys[j];
    keys[i] = y->keys[t - 1];
    n = n + 1;
}

void BTreeNode::insertNonFull(int k) {
    int i = n - 1;
    if (leaf) {
        while (i >= 0 && keys[i] > k) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = k;
        n = n + 1;
    } else {
        while (i >= 0 && keys[i] > k) i--;
        if (children[i + 1]->n == 2 * t - 1) {
            splitChild(i + 1, children[i + 1]);
            if (keys[i + 1] < k) i++;
        }
        children[i + 1]->insertNonFull(k);
    }
}

class BTree {
public:
    BTreeNode *root;
    int t;

    BTree() {
        root = nullptr;
        t = 2; // order 4
    }

    void insert(int k) {
        if (root == nullptr) {
            root = new BTreeNode(t, true);
            root->keys[0] = k;
            root->n = 1;
        } else {
            if (root->n == 2 * t - 1) {
                BTreeNode *s = new BTreeNode(t, false);
                s->children[0] = root;
                s->splitChild(0, root);
                int i = 0;
                if (s->keys[0] < k) i++;
                s->children[i]->insertNonFull(k);
                root = s;
            } else {
                root->insertNonFull(k);
            }
        }
    }

    bool search(int k) {
        if (root == nullptr) return false;
        return root->search(k);
    }

    void levelOrder(ostream& out = cout) {
        if (root == nullptr) {
            out << "Tree is empty." << endl;
            return;
        }
        BTreeNode* currentLevel[1000];
        BTreeNode* nextLevel[1000];
        int currentCount = 0;
        int nextCount = 0;

        currentLevel[currentCount++] = root;
        int level = 0;

        while (currentCount > 0) {
            out << "Level " << level << ": ";
            for (int i = 0; i < currentCount; i++) {
                BTreeNode* node = currentLevel[i];
                out << "[";
                for (int j = 0; j < node->n; j++) {
                    out << node->keys[j];
                    if (j < node->n - 1) out << " ";
                }
                out << "] ";
                if (!node->leaf) {
                    for (int j = 0; j <= node->n; j++) {
                        nextLevel[nextCount++] = node->children[j];
                    }
                }
            }
            out << endl;
            for (int i = 0; i < nextCount; i++) currentLevel[i] = nextLevel[i];
            currentCount = nextCount;
            nextCount = 0;
            level++;
        }
    }

    void clearHelper(BTreeNode* node) {
        if (node != nullptr) {
            if (!node->leaf) {
                for (int i = 0; i <= node->n; i++) clearHelper(node->children[i]);
            }
            delete[] node->keys;
            delete[] node->children;
            delete node;
        }
    }

    void clear() {
        clearHelper(root);
        root = nullptr;
    }

    void save(string filename) {
        ofstream out(filename);
        if (root != nullptr) root->serialize(out);
        out.close();
    }

    BTreeNode* deserializeHelper(ifstream& in) {
        bool isLeaf;
        int numKeys;
        if (!(in >> isLeaf >> numKeys)) return nullptr; 
        BTreeNode* node = new BTreeNode(t, isLeaf);
        node->n = numKeys;
        for (int i = 0; i < numKeys; i++) in >> node->keys[i];
        if (!isLeaf) {
            for (int i = 0; i <= numKeys; i++) node->children[i] = deserializeHelper(in);
        }
        return node;
    }

    void restore(string filename) {
        ifstream in(filename);
        if (!in) return;
        clear(); 
        root = deserializeHelper(in);
        in.close();
    }

    int getHeight() {
        if (root == nullptr) return 0;
        return root->getHeight();
    }
};

int main() {
    cout << "Section 1: File I/O & Logging" << endl;
    
    BTree fileTree;
    totalSplits = 0; // reset for accurate count

    ifstream inFile("input.txt");
    ofstream logFile("log.txt");
    ofstream outFile("output.txt");

    if (inFile) {
        logFile << "step-by-step log:" << endl;
        string line;
        
        while (getline(inFile, line)) {
            if (line.empty()) continue; 
            
            stringstream ss(line);
            char op;
            int val;
            
            if (ss >> op >> val) {
                if (op == 'I') {
                    logFile << "Insert " << val << ":" << endl;
                    fileTree.insert(val);
                    fileTree.levelOrder(logFile);
                } else if (op == 'S') {
                    bool found = fileTree.search(val);
                    logFile << "Search " << val << ": " << (found ? "Found" : "Not Found") << endl;
                }
            }
        }

        outFile << "Final Tree Structure:" << endl;
        fileTree.levelOrder(outFile);
        outFile << "total splits = " << totalSplits << endl;
        logFile << "total splits = " << totalSplits << endl;

        inFile.close();
        logFile.close();
        outFile.close();
        cout << "File processing complete! Check log.txt and output.txt.\n\n";
    } else {
        cout << "Warning: input.txt not found. Skipping file section.\n\n";
    }

    cout << "Section 2: Reverse Reconstruction" << endl;
    
    int arr[] = {10, 20, 30, 40, 50, 60, 70}; 
    int n = 7;

    BTree treeNormal;
    totalSplits = 0; 
    for(int i = 0; i < n; i++) treeNormal.insert(arr[i]);
    cout << "Normal (10 to 70) Height: " << treeNormal.getHeight() << " | Splits: " << totalSplits << endl;
    treeNormal.levelOrder();

    BTree treeReverse;
    totalSplits = 0; 
    for(int i = n - 1; i >= 0; i--) treeReverse.insert(arr[i]);
    cout << "\nReverse (70 to 10) Height: " << treeReverse.getHeight() << " | Splits: " << totalSplits << endl;
    treeReverse.levelOrder();
    cout << "\n";


    cout << "Section 3: System Recovery" << endl;
    
    cout << "Saving Normal tree to snapshot.dat:" << endl;
    treeNormal.save("snapshot.dat");
    
    BTree restoredTree;
    restoredTree.restore("snapshot.dat");
    cout << "Restored Tree Output (Should match Normal Tree above):" << endl;
    restoredTree.levelOrder();
    cout << "\n";


    cout << "Section 4: Extra Challenge" << endl;
    
    BTree extraTree;
    totalSplits = 0;
    int extraArr[] = {100, 90, 80, 70, 60, 50, 40, 30, 20, 10};
    
    cout << "Inserting 100 down to 10:" << endl;
    for(int i = 0; i < 10; i++) {
        extraTree.insert(extraArr[i]);
    }
    
    extraTree.levelOrder();
    cout << "Extra Challenge Height: " << extraTree.getHeight() << endl;
    cout << "Extra Challenge Splits: " << totalSplits << endl;

    return 0;
}
