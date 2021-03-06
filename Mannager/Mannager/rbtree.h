#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#pragma warning(disable:4996)
#define MAX_NAME 21
#define SIZE 20
typedef unsigned long long ULL;
enum rb_color
{
	RB_BLACK,
	RB_RED,
};

typedef struct rbtree_node
{
	struct rbtree_node* parent;
	struct rbtree_node* left;
	struct rbtree_node* right;
	enum rb_color color;
	void*  key;
	void *data;
}rbtree_node;

typedef int(*rbtree_cmp_fn_t)(void *key_a, void *key_b);
typedef void(*ptdata)(void *data, FILE *stream);
typedef void(*rtdata)(void*data, FILE *stream);
typedef struct rbtree
{
	struct rbtree_node* root;
	rbtree_cmp_fn_t compare;
}rbtree;
void padding(char ch, int n)
{
	int i;

	for (i = 0; i < n; i++)
		putchar(ch);
}

static inline void set_color(enum rb_color color, struct rbtree_node* node)
{
	assert(node != NULL);
	node->color = color;
}

struct rbtree_node* sibling(rbtree_node* node)
{
	assert(node != NULL);
	assert(node->parent != NULL); /* Root node has no sibling */
	if (node == node->parent->left)
		return node->parent->right;
	else
		return node->parent->left;
}
static inline rbtree_node* get_min(struct rbtree_node* node)
{
	assert(node != NULL);
	while (node->left)
	{
		node = node->left;
	}
	return node;
}

struct rbtree_node* rbtree_min(struct rbtree *tree)
{
	if (tree->root == NULL)
		return NULL;
	else
	{
		return get_min(tree->root);
	}
}

struct rbtree_node* rbtree_createnode(void *key, void* data)
{
	struct rbtree_node* newnode = (struct rbtree_node*)malloc(sizeof(struct rbtree_node));
	if (newnode == NULL)
		return NULL;

	newnode->key = key;
	newnode->data = data;
	newnode->parent = NULL;
	newnode->left = NULL;
	newnode->right = NULL;
	return newnode;
}

struct rbtree_node* do_lookup(void* key, struct rbtree* tree, struct rbtree_node** pparent)
{
	struct rbtree_node *current = tree->root;

	while (current)
	{
		int ret = tree->compare(current->key, key);
		if (ret == 0)
			return  current;
		else
		{
			if (pparent != NULL)
			{
				*pparent = current;
			}
			if (ret < 0)
				current = current->right;
			else
				current = current->left;
		}
	}
	return NULL;

}

void*  rbtree_lookup(struct rbtree* tree, void* key)
{
	assert(tree != NULL);
	struct rbtree_node* node;
	node = do_lookup(key, tree, NULL);
	return node == NULL ? NULL : node->data;
}

static void set_child(struct rbtree* tree, struct rbtree_node* node, struct rbtree_node* child)
{
	int ret = tree->compare(node->key, child->key);
	assert(ret != 0);

	if (ret > 0)
	{
		node->left = child;
	}
	else {
		node->right = child;
	}
}

static void rotate_left(struct rbtree_node* node, struct rbtree* tree)
{
	rbtree_node*x = node, *y = node->right;
	x->right = y->left;
	if (y->left != NULL)
		y->left->parent = x;
	y->parent = x->parent;
	if (x->parent == NULL)
		tree->root = y;
	else
	{
		if (x == x->parent->left)
			x->parent->left = y;
		else
			x->parent->right = y;
	}
	y->left = x;
	x->parent = y;
}

static void rotate_right(struct rbtree_node *node, struct rbtree *tree)
{
	rbtree_node*y = node, *x = node->left;
	y->left = x->right;
	if (x->right != NULL)
		x->right->parent = y;
	x->parent = y->parent;
	if (y->parent == NULL)
		tree->root = x;
	else
	{
		if (y == y->parent->left)
			y->parent->left = x;
		else
			y->parent->right = x;
	}
	x->right = y;
	y->parent = x;
}

struct rbtree* rbtree_init(rbtree_cmp_fn_t compare)
{
	struct rbtree* tree = (struct rbtree*)malloc(sizeof(struct rbtree));
	if (tree == NULL)
		return NULL;
	else
	{
		tree->root = NULL;
		tree->compare = compare;
	}

	return tree;
}
struct rbtree_node* __rbtree_insert(struct rbtree_node* node, struct rbtree *tree)
{
	rbtree_node*parent = NULL, *x, *y;
	rbtree_node*result = do_lookup(node->key, tree, &parent);
	node->color = RB_RED;
	if (result) return result;
	if (parent == NULL) tree->root = node;
	else
		set_child(tree, parent, node);
	node->parent = parent;
	x = node;
	while (x->parent != NULL && x->parent->color == RB_RED)
	{
		if (x->parent == x->parent->parent->left)
		{
			y = x->parent->parent->right;
			if (y && y->color == RB_RED)
			{
				x->parent->color = RB_BLACK;
				x->parent->parent->color = RB_RED;
				y->color = RB_BLACK;
				x = x->parent->parent;
			}
			else
			{
				if (x == x->parent->right)
				{
					x = x->parent;
					rotate_left(x, tree);
				}
				x->parent->color = RB_BLACK;
				x->parent->parent->color = RB_RED;
				rotate_right(x->parent->parent, tree);
			}
		}
		else
		{
			y = x->parent->parent->left;
			if (y && y->color == RB_RED)
			{
				x->parent->color = RB_BLACK;
				x->parent->parent->color = RB_RED;
				y->color = RB_BLACK;
				x = x->parent->parent;
			}
			else
			{
				if (x == x->parent->left)
				{
					x = x->parent;
					rotate_right(x, tree);
				}
				x->parent->color = RB_BLACK;
				x->parent->parent->color = RB_RED;
				rotate_left(x->parent->parent, tree);
			}
		}
	}
	tree->root->color = RB_BLACK;
	return NULL;
}

int  rbtree_insert(struct rbtree *tree, void*  key, void* data)
{
	struct rbtree_node * node = rbtree_createnode(key, data);
	struct rbtree_node* samenode = NULL;
	if (node == NULL)
		return -1;
	else
		samenode = __rbtree_insert(node, tree);
	if (samenode != NULL)
		return -2;
	return 0;
}

void rbtree_balance(rbtree*tree, rbtree_node*x, rbtree_node*x_parent)
{
	while (x != tree->root && (x == 0 || x->color == RB_BLACK))
	{
		if (x == x_parent->left)
		{
			rbtree_node*w = x_parent->right;
			if (w->color == RB_RED)
			{
				w->color = RB_BLACK;
				x_parent->color = RB_RED;
				rotate_left(x_parent, tree);
				w = x_parent->right;
			}
			if ((w->left == 0 || w->left->color == RB_BLACK) && (w->right == 0 || w->right->color == RB_BLACK))
			{
				w->color = RB_RED;
				x = x_parent;
				x_parent = x_parent->parent;
			}
			else
			{
				if (w->right == 0 || w->right->color == RB_BLACK)
				{
					if (w->left) w->left->color = RB_BLACK;
					w->color = RB_RED;
					rotate_right(w, tree);
					w = x_parent->right;
				}
				w->color = x_parent->color;
				x_parent->color = RB_BLACK;
				if (w->right) w->right->color = RB_BLACK;
				rotate_left(x_parent, tree);
				break;
			}
		}
		else
		{
			rbtree_node*w = x_parent->left;
			if (w->color == RB_RED)
			{
				w->color = RB_BLACK;
				x_parent->color = RB_RED;
				rotate_right(x_parent, tree);
				w = x_parent->left;
			}
			if ((w->left == 0 || w->left->color == RB_BLACK) && (w->right == 0 || w->right->color == RB_BLACK))
			{
				w->color = RB_RED;
				x = x_parent;
				x_parent = x_parent->parent;
			}
			else
			{
				if (w->left == 0 || w->left->color == RB_BLACK)
				{
					if (w->right) w->right->color = RB_BLACK;
					w->color = RB_RED;
					rotate_left(w, tree);
					w = x_parent->left;
				}
				w->color = x_parent->color;
				x_parent->color = RB_BLACK;
				if (w->left) w->left->color = RB_BLACK;
				rotate_right(x_parent, tree);
				break;
			}
		}
	}
	if (x) x->color = RB_BLACK;
}

void _rbtree_remove(rbtree_node* z, struct rbtree* tree)
{
	rbtree_node*y, *x, *parent;
	y = z;
	enum rb_color temp;
	if (y->left == NULL)
		x = y->right;
	else if (y->right == NULL)
		x = y->left;
	else
	{
		y = get_min(y->right);
		x = y->right;
	}
	if (y != z)
	{
		z->left->parent = y;
		y->left = z->left;
		if (y != z->right) {
			parent = y->parent;
			if (x)x->parent = y->parent;
			y->parent->left = x;
			y->right = z->right;
			z->right->parent = y;
		}
		else
			parent = y;
		if (tree->root == z)
			tree->root = y;
		else if (z->parent->left == z)
			z->parent->left = y;
		else
			z->parent->right = y;
		y->parent = z->parent;
		temp = y->color; y->color = z->color; z->color = temp;
		y = z;
	}
	else
	{
		parent = y->parent;
		if (x) x->parent = y->parent;
		if (tree->root == z)
			tree->root = x;
		else
			if (z->parent->left == z)
				z->parent->left = x;
			else
				z->parent->right = x;
	}
	if (y->color != RB_RED)
		rbtree_balance(tree, x, parent);
}
int  rbtree_remove(struct rbtree* tree, void *key)
{
	struct rbtree_node* node = do_lookup(key, tree, NULL);
	if (node == NULL)
		return -1;
	else
		_rbtree_remove(node, tree);
	return 0;
}


void print_node(struct rbtree_node *root, int level)
{

	if (root == NULL)
	{
		padding('\t', level);
		puts("NIL");

	}
	else
	{
		print_node(root->right, level + 1);
		padding('\t', level);
		if (root->color == RB_BLACK)
		{
			printf("(%s)\n", (char*)(root->key));
		}
		else
			printf("%s\n", (char*)(root->key));
		print_node(root->left, level + 1);
	}
}

void print_tree(struct rbtree* tree)
{
	print_node(tree->root, 0);
	printf("-------------------------------------------\n");
}


void  process_null_node(struct rbtree_node* node, FILE* stream)
{
	fprintf(stream, "0\n");
}
void __tree2dot(struct rbtree_node* node, FILE* stream, ptdata pt)
{
	fprintf(stream, "1\n");
	if (node->color == RB_BLACK)
		fprintf(stream, "1\n");    //0 r  1 b
	else
		fprintf(stream, "0\n");
	pt(node, stream);
	if (node->left)
		__tree2dot(node->left, stream, pt);
	else
	{
		process_null_node(node, stream);
	}
	if (node->right)
		__tree2dot(node->right, stream, pt);
	else
		process_null_node(node, stream);
}
int tree2dot(struct rbtree* tree, char* filename, ptdata pt)
{
	assert(tree != NULL && filename != NULL);
	FILE* stream = fopen(filename, "w+");
	if (stream == NULL)
	{
		fprintf(stderr, "open failed \n");
		return -1;
	}
	if (!tree->root) { fclose(stream); return 0; }
	__tree2dot(tree->root, stream, pt);
	fclose(stream);
	return 0;

}

void __treefdot(struct rbtree_node* pre, int LorR, FILE* stream, rtdata rt)
{
	int con, color;
	fscanf(stream, "%d", &con);
	if (!con) return;
	fscanf(stream, "%d", &color);
	struct rbtree_node* newnode = (struct rbtree_node*)malloc(sizeof(struct rbtree_node));
	newnode->left = NULL;
	newnode->right = NULL;
	if (color)
		set_color(RB_BLACK, newnode);
	else
		set_color(RB_RED, newnode);
	newnode->parent = pre;
	if (LorR == -1)
		pre->left = newnode;
	else if (LorR == 1)
		pre->right = newnode;
	rt(newnode, stream);
	__treefdot(newnode, -1, stream, rt);
	__treefdot(newnode, 1, stream, rt);
}
int __rtreefdot(struct rbtree_node* root, FILE* stream, rtdata rt)
{
	int con, color;
	if (fscanf(stream, "%d", &con)==EOF) return 0;
	if (!con) return 1;
	fscanf(stream, "%d", &color);
	root->parent = NULL;
	root->left = NULL;
	root->right = NULL;
	if (color)
		set_color(RB_BLACK, root);
	else
		set_color(RB_RED, root);
	rt(root, stream);
	__treefdot(root, -1, stream, rt);
	__treefdot(root, 1, stream, rt);
}
int treefdot(struct rbtree* tree, char* filename, rtdata rt)
{
	assert(tree != NULL && filename != NULL);
	FILE* stream = fopen(filename, "r");
	if (stream == NULL)
	{
		fprintf(stderr, "open failed \n");
		return -1;
	}
	tree->root = (rbtree_node*)malloc(sizeof(rbtree_node));
	if (!__rtreefdot(tree->root, stream, rt)) {
		free(tree->root); 
		tree->root = NULL;
	}
	fclose(stream);
	return 0;

}

