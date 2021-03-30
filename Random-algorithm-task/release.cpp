#include <iostream>
#include <algorithm>
#include <tuple>

struct Node {
    int size;
    long long value;
    long long sum;

    bool isReversed;
    bool isAssigned;
    long long add;

    int maxNonIncreasingSuffix;
    int maxNonDecreasingPrefix;
    int maxNonDecreasingSuffix;
    int maxNonIncreasingPrefix;
    long long leftest;
    long long rightest;
    
    Node* left;
    Node* right;
    Node* parent;

    Node(long long value)
        : size(1), value(value), sum(value), isReversed(false),
        isAssigned(false), add(0),
        maxNonIncreasingSuffix(1), maxNonDecreasingPrefix(1),
        maxNonDecreasingSuffix(1), maxNonIncreasingPrefix(1),
        leftest(value), rightest(value),
        left(nullptr), right(nullptr), parent(nullptr)
    {   }
};


int getSize(Node* node) {
    if (node == nullptr)
        return 0;
    return node->size;
}

long long getSum(Node* node) {
    if (node == nullptr)
        return 0;
    return node->sum;
}

int getMaxNonIncreasingSuffix(Node* node) {
    if (node == nullptr)
        return 0;
    return node->maxNonIncreasingSuffix;
}

int getMaxNonDecreasingPrefix(Node* node) {
    if (node == nullptr)
        return 0;
    return node->maxNonDecreasingPrefix;
}

int getMaxNonDecreasingSuffix(Node* node) {
    if (node == nullptr)
        return 0;
    return node->maxNonDecreasingSuffix;
}

int getMaxNonIncreasingPrefix(Node* node) {
    if (node == nullptr)
        return 0;
    return node->maxNonIncreasingPrefix;
}





void toggleReversed(Node* node) {
    if (node != nullptr)
        node->isReversed ^= true;
}

void lazyAdd(Node* node, long long value) {
    if (node != nullptr)
        node->add += value;
}

void setAssigned(Node* node) {
    if (node != nullptr) {
        node->isAssigned = true;
        node->add = 0;
    }
}

void push(Node* node) {
    if (node == nullptr)
        return;
    if (node->isReversed) {
        node->isReversed = false;
        std::swap(node->maxNonIncreasingSuffix, node->maxNonDecreasingPrefix);
        std::swap(node->maxNonDecreasingSuffix, node->maxNonIncreasingPrefix);
        std::swap(node->leftest, node->rightest);
        std::swap(node->left, node->right);
        toggleReversed(node->left);
        toggleReversed(node->right);
    }
    
    if (node->isAssigned) {
        node->value = 0;
        node->sum = 0;
        
        node->maxNonIncreasingSuffix = node->size;
        node->maxNonDecreasingPrefix = node->size;
        node->maxNonDecreasingSuffix = node->size;
        node->maxNonIncreasingPrefix = node->size;

        node->leftest = 0;
        node->rightest = 0;

        setAssigned(node->left);
        setAssigned(node->right);

        node->isAssigned = false;
    }

    if (node->add != 0) {
        node->value += node->add;
        node->sum += node->size * node->add;
        
        node->leftest += node->add;
        node->rightest += node->add;
        
        lazyAdd(node->left, node->add);
        lazyAdd(node->right, node->add);

        node->add = 0;
    }
}


void update(Node* node) {
    if (node == nullptr)
        return;

    push(node);
    push(node->left);
    push(node->right);

    node->size = getSize(node->left) + getSize(node->right) + 1;
    node->sum = getSum(node->left) + getSum(node->right) + node->value;
    
    if (node->left == nullptr)
        node->leftest = node->value;
    else
        node->leftest = node->left->leftest;

    if (node->right == nullptr)
        node->rightest = node->value;
    else
        node->rightest = node->right->rightest;
    
    

    node->maxNonIncreasingSuffix = getMaxNonIncreasingSuffix(node->right);

    if (getMaxNonIncreasingSuffix(node->right) == getSize(node->right)) {
        if (node->right == nullptr || node->value >= node->right->leftest) {
            ++node->maxNonIncreasingSuffix;

            if (node->left != nullptr && node->left->rightest >= node->value)
                node->maxNonIncreasingSuffix += node->left->maxNonIncreasingSuffix; 
        }
    }

    

    node->maxNonDecreasingPrefix = getMaxNonDecreasingPrefix(node->left);

    if (getMaxNonDecreasingPrefix(node->left) == getSize(node->left)) {
        if (node->left == nullptr || node->value >= node->left->rightest) {
            ++node->maxNonDecreasingPrefix;

            if (node->right != nullptr && node->right->leftest >= node->value)
                node->maxNonDecreasingPrefix += node->right->maxNonDecreasingPrefix; 
        }
    }



    
    node->maxNonDecreasingSuffix = getMaxNonDecreasingSuffix(node->right);

    if (getMaxNonDecreasingSuffix(node->right) == getSize(node->right)) {
        if (node->right == nullptr || node->value <= node->right->leftest) {
            ++node->maxNonDecreasingSuffix;

            if (node->left != nullptr && node->left->rightest <= node->value)
                node->maxNonDecreasingSuffix += node->left->maxNonDecreasingSuffix; 
        }
    }



    node->maxNonIncreasingPrefix = getMaxNonIncreasingPrefix(node->left);

    if (getMaxNonIncreasingPrefix(node->left) == getSize(node->left)) {
        if (node->left == nullptr || node->value <= node->left->rightest) {
            ++node->maxNonIncreasingPrefix;

            if (node->right != nullptr && node->right->leftest <= node->value)
                node->maxNonIncreasingPrefix += node->right->maxNonIncreasingPrefix; 
        }
    }
}

void setParent(Node* child, Node* parent) {
    if (child != nullptr)
        child->parent = parent;
}

void leftRotation(Node* p) {
    Node* x = p->right;
    Node* b = x->left;
    Node* c = x->right;
    Node* a = p->left;
    Node* g = p->parent;

    setParent(b, p);
    p->right = b;

    setParent(p, x);
    x->left = p;

    setParent(x, g);

    if (g != nullptr) {
        if (p == g->left)
            g->left = x;
        else
            g->right = x;
    }

    update(p);
    update(x);
    update(g);
}

void rightRotation(Node* p) {
    Node* x = p->left;
    Node* a = x->left;
    Node* b = x->right;
    Node* c = p->right;
    Node* g = p->parent;

    setParent(b, p);
    p->left = b;

    setParent(p, x);
    x->right = p;

    setParent(x, g);

    if (g != nullptr) {
        if (p == g->left)
            g->left = x;
        else
            g->right = x;
    }

    update(p);
    update(x);
    update(g);
}


void leftZig(Node* node) {
    rightRotation(node->parent);
}

void rightZig(Node* node) {
    leftRotation(node->parent);
}

void leftZigZig(Node* node) {
    rightRotation(node->parent->parent);
    rightRotation(node->parent);
}

void rightZigZig(Node* node) {
    leftRotation(node->parent->parent);
    leftRotation(node->parent);
}

void leftZigZag(Node* node) { 
    rightRotation(node->parent);
    leftRotation(node->parent);
}

void rightZigZag(Node* node) {
    leftRotation(node->parent);
    rightRotation(node->parent);
}


void splay(Node* node) {
    if (node == nullptr || node->parent == nullptr)
        return;

    if (node->parent->parent == nullptr) {
        if (node == node->parent->left)
            leftZig(node);
        else
            rightZig(node);
    } else {
        if (node == node->parent->left) {
            if (node->parent == node->parent->parent->left)
                leftZigZig(node);
            else
                leftZigZag(node);
        } else {
            if (node->parent == node->parent->parent->right)
                rightZigZig(node);
            else
                rightZigZag(node);
        }
        splay(node);
    }
}

Node* get(Node* node, int index) {
    if (node == nullptr)
        return nullptr;

    push(node);
    push(node->left);
    push(node->right);

    if (getSize(node->left) == index) {
        splay(node);
        return node;
    } else if (getSize(node->left) > index) { 
        return get(node->left, index);
    } else {
        return get(node->right, index - getSize(node->left) - 1);
    }
}


Node* merge(Node* left, Node* right) {
    push(left);
    push(right);

    if (left == nullptr)
        return right;
    if (right == nullptr)
        return left;
    
    Node* newRoot = get(right, 0);

    setParent(left, newRoot);
    newRoot->left = left;

    update(newRoot); 
    
    return newRoot;
}

std::pair<Node*, Node*> split(Node* tree, int index) {
    if (tree == nullptr)
        return std::make_pair(nullptr, nullptr);
    
    push(tree);

    if (index == -1)
        return std::make_pair(nullptr, tree);

    Node* left = get(tree, index);
    Node* right = left->right;

    setParent(right, nullptr);
    left->right = nullptr;
    update(left);    
    
    return std::make_pair(left, right);
}

Node* insert(Node* tree, Node* x, int index) {
    Node* left;
    Node* right;

    std::tie(left, right) = split(tree, index - 1);
    setParent(left, x);
    setParent(right, x);

    x->left = left;
    x->right = right;
    
    update(x);

    return x;
}

Node* removeRoot(Node* x) {
    push(x);

    Node* left = x->left;
    Node* right = x->right;

    setParent(left, nullptr);
    setParent(right, nullptr);

    delete x;

    return merge(left, right);
}

std::tuple<Node*, Node*, Node*> splitOutSubsegment(Node* tree, int l, int r) {
    Node* left;
    Node* mid;
    Node* right;

    std::tie(left, right) = split(tree, l - 1);
    std::tie(mid, right) = split(right, r - l);

    return std::make_tuple(left, mid, right);
}

Node* splitInSubsegment(Node* left, Node* mid, Node* right) {
    left = merge(left, mid);
    return merge(left, right);
}

std::pair<Node*, long long> rsq(Node* tree, int l, int r) {
    Node* left;
    Node* mid;
    Node* right;

    std::tie(left, mid, right) = splitOutSubsegment(tree, l, r);

    long long result = getSum(mid);

    return std::make_pair(splitInSubsegment(left, mid, right), result);
}

Node* assign(Node* tree, int l, int r, long long value) {
    Node* left;
    Node* mid;
    Node* right;

    std::tie(left, mid, right) = splitOutSubsegment(tree, l, r);

    setAssigned(mid);
    lazyAdd(mid, value);

    return splitInSubsegment(left, mid, right);
}

Node* add(Node* tree, int l, int r, int value) {
    Node* left;
    Node* mid;
    Node* right;

    std::tie(left, mid, right) = splitOutSubsegment(tree, l, r);

    lazyAdd(mid, value);

    return splitInSubsegment(left, mid, right);
}

Node* majOnNonIncreasing(Node* node, long long value) {
    if (node == nullptr)
        return nullptr;
    
    push(node);
    push(node->left);
    push(node->right);

    if (node->value <= value) {
        return majOnNonIncreasing(node->left, value);
    } else if (node->right == nullptr || node->right->leftest <= value) {
        splay(node);
        return node;
    } else {
        return majOnNonIncreasing(node->right, value);
    }
}

Node* minOnNonDecreasing(Node* node, long long value) {
    if (node == nullptr)
        return nullptr;

    push(node);
    push(node->left);
    push(node->right);
    
    if (node->value >= value) {
        return minOnNonDecreasing(node->left, value);
    } else if (node->right == nullptr || node->right->leftest >= value) {
        splay(node);
        return node;
    } else {
        return minOnNonDecreasing(node->right, value);
    }
}

Node* nextPermutation(Node* tree, int l, int r) {
    if (tree == nullptr)
        return nullptr;
    
    Node* left;
    Node* mid;
    Node* right;

    std::tie(left, mid, right) = splitOutSubsegment(tree, l, r);
    
    int suffixSize = getMaxNonIncreasingSuffix(mid);

    if (suffixSize == getSize(mid)) {
        toggleReversed(mid);
        return splitInSubsegment(left, mid, right);        
    }

    Node* suffix;

    std::tie(mid, suffix) = split(mid, getSize(mid) - suffixSize - 1);
    suffix = majOnNonIncreasing(suffix, mid->value);
    
    std::swap(mid->value, suffix->value);
    update(mid);
    update(suffix);
    toggleReversed(suffix);

    mid = merge(mid, suffix);

    return splitInSubsegment(left, mid, right);
}

Node* prevPermutation(Node* tree, int l, int r) {
    if (tree == nullptr)
        return nullptr;

    Node* left;
    Node* mid;
    Node* right;

    std::tie(left, mid, right) = splitOutSubsegment(tree, l, r);

    int suffixSize = getMaxNonDecreasingSuffix(mid);

    if (suffixSize == getSize(mid)) {
        toggleReversed(mid);
        return splitInSubsegment(left, mid, right);
    }

    Node* suffix;

    std::tie(mid, suffix) = split(mid, getSize(mid) - suffixSize - 1);
    suffix = minOnNonDecreasing(suffix, mid->value);

    std::swap(mid->value, suffix->value);
    update(mid);
    update(suffix);
    toggleReversed(suffix);

    mid = merge(mid, suffix);

    return splitInSubsegment(left, mid, right);
}


int main() {
    std::ios_base::sync_with_stdio(false);

    int n;
    std::cin >> n;

    Node* tree = nullptr;
    
    int counter = 0;
    for (counter = 1; counter <= n; ++counter) {
        long long x;
        std::cin >> x;

        Node* newNode = new Node(x);
        tree = merge(tree, newNode); 
    }

    int q;
    std::cin >> q;

    while (q--) {
        int com;
        std::cin >> com;

        switch(com) {
          case 1: {
            int l;
            int r;
            std::cin >> l >> r;
            long long sum;
            std::tie(tree, sum) = rsq(tree, l, r);

            std::cout << sum << '\n';

            break;
          }
          case 2: {
            long long value;
            int pos;
            std::cin >> value >> pos;

            Node* newNode = new Node(value);
            tree = insert(tree, newNode, pos);

            break;
          }
          case 3: {
            int pos;
            std::cin >> pos;

            tree = get(tree, pos);
            tree = removeRoot(tree);

            break;
          }
          case 4: {
            long long value;
            int l;
            int r;

            std::cin >> value >> l >> r;

            tree = assign(tree, l, r, value);

            break;
          }
          case 5: {
            long long value;
            int l;
            int r;

            std::cin >> value >> l >> r;

            tree = add(tree, l, r, value);

            break;
          }
          case 6: {
            int l;
            int r;

            std::cin >> l >> r;

            tree = nextPermutation(tree, l, r);
            
            break;
          }
          case 7: {
            int l;
            int r;

            std::cin >> l >> r;

            tree = prevPermutation(tree, l, r);

            break;
          }
        }
    }

    for (int i = 0; i < getSize(tree); ++i) {
        tree = get(tree, i);
        std::cout << tree->value << ' '; 
    }
    std::cout << '\n';
    
    return 0;
}
