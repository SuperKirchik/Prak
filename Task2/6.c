#include <stdio.h>
#include <stdlib.h>

typedef struct TreeNode {
    int key;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* create_node(int key) {
    TreeNode* new_node = (TreeNode*)malloc(sizeof(TreeNode));
    new_node->key = key;
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

TreeNode* insert(TreeNode* root, int key) {
    if (root == NULL) {
        return create_node(key);
    }
    
    if (key < root->key) {
        root->left = insert(root->left, key);
    } else if (key > root->key) {
        root->right = insert(root->right, key);
    }
    
    return root;
}

TreeNode* min(TreeNode* root) {
    while (root != NULL && root->left != NULL) {
        root = root->left;
    }
    return root;
}

TreeNode* delete(TreeNode* root, int key) {
    if (root == NULL) {
        return NULL;
    }
    
    if (key < root->key) {
        root->left = delete(root->left, key);
    } else if (key > root->key) {
        root->right = delete(root->right, key);
    } else {
        if (root->left == NULL) {
            TreeNode* temp = root->right;
            free(root);
            return temp;
        } else if (root->right == NULL) {
            TreeNode* temp = root->left;
            free(root);
            return temp;
        }
        
        TreeNode* temp = min(root->right);
        root->key = temp->key;
        root->right = delete(root->right, temp->key);
    }
    
    return root;
}

int find_el(TreeNode* root, int key) {
    if (root == NULL) {
        return 0;
    }
    
    if (key < root->key) {
        return find_el(root->left, key);
    } else if (key > root->key) {
        return find_el(root->right, key);
    } else {
        return 1;
    }
}


int main() {
    TreeNode* root = NULL;
    char operation;
    int number;
    
    while (scanf(" %c%d", &operation, &number) != EOF) {
        switch (operation) {
            case '+':
                root = insert(root, number);
                break;
            case '-':
                root = delete(root, number);
                break;
            case '?':
                printf("%d %s\n", number, find_el(root, number) ? "yes" : "no");
                break;
        }
    }
    
    free_tree(root);
    return 0;
}