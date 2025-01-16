#include <iostream>
#include <stack>
#include <vector>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

vector<int> pre;
vector<int> bac;

void dfs(TreeNode* root) {
    if (!root) return;
    stack<TreeNode*> s;
    stack<bool> visited;
    s.push(root);
    visited.push(false);

    while (!s.empty()) {
        TreeNode* node = s.top();
        bool isVisited = visited.top();
        s.pop();
        visited.pop();

        if (isVisited) {
            bac.push_back(node->val);
        } else {
            s.push(node);
            visited.push(true);
            pre.push_back(node->val);
            if (node->right) {
                s.push(node->right);
                visited.push(false);
            }
            if (node->left) {
                s.push(node->left);
                visited.push(false);
            }
        }
    }
}

int main() {
    TreeNode* root = new TreeNode(1);
    root->left = new TreeNode(2);
    root->right = new TreeNode(3);
    root->left->left = new TreeNode(4);
    root->left->right = new TreeNode(5);
    root->right->left = new TreeNode(6);
    root->right->right = new TreeNode(7);

    std::cout << "DFS Traversal: ";
    dfs(root);
    std::cout << std::endl;

    for(auto i : pre) {
        std::cout << i << " ";
    }

    cout << endl;

    for(auto i : bac) {
        std::cout << i << " ";
    }

    // Clean up memory
    delete root->right->right;
    delete root->right->left;
    delete root->right;
    delete root->left->right;
    delete root->left->left;
    delete root->left;
    delete root;

    return 0;
}