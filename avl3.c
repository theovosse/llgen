#include <stdio.h>
#include <stdlib.h>
#include "storage.h"
#include "avl3.h"
#include "bool.h"

struct tree
{
	void *key;
	void *info;
	TREE left;
	TREE right;
	TREE next;
	TREE previous;
	int depth;
};

#define Depth(node) (node == NULL? 0: node->depth)

static TREE oldNodes = NULL;

static int NDepth(TREE t)
{
	int lDepth;
	int rDepth;
	
	if (t == NULL)
	{
		return (0);
	}
	lDepth = Depth(t->left);
	rDepth = Depth(t->right);
	return ((lDepth > rDepth? lDepth: rDepth) + 1);
}

static TREE InsertNodeSingle2(TREE *tree, void *key, void *info, int (*keyCmp)(void *, void *), TREE previous, TREE next)
{
	int lDepth;
	int rDepth;
	TREE n0 = *tree;
	TREE n1;
	TREE n2;
	int res;
	TREE ret;
	
	if (n0 == NULL)
	{
		if (oldNodes == NULL)
		{
			n0 = (TREE) mmalloc(sizeof(struct tree));
			if (n0 == NULL)
			{
				perror("AVL Insert");
				exit(1);
			}
		}
		else
		{
			n0 = oldNodes;
			oldNodes = oldNodes->left;
		}
		n0->key = key;
		n0->info = info;
		n0->left = n0->right = NULL;
		n0->depth = 1;
		if (next != NULL)
		{
			n0->previous = next->previous;
			if (next->previous != NULL)
			{
				next->previous->next = n0;
			}
			next->previous = n0;
			n0->next = next;
		}
		else if (previous != NULL)
		{
			n0->next = previous->next;
			if (previous->next != NULL)
			{
				previous->next->previous = n0;
			}
			previous->next = n0;
			n0->previous = previous;
		}
		else
		{
			n0->next = NULL;
			n0->previous = NULL;
		}
		*tree = n0;
		ret = n0;
	}
	else
	{
		if ((res = (*keyCmp)(n0->key, key)) > 0)
		{
			ret = InsertNodeSingle2(&(n0->left), key, info, keyCmp, NULL, n0);
		}
		else if (res < 0)
		{
			ret = InsertNodeSingle2(&(n0->right), key, info, keyCmp, n0, NULL);
		}
		else
		{
			ret = n0;
		}
		if (n0->left == NULL)
		{
			lDepth = 0;
		}
		else
		{
			lDepth = n0->left->depth;
		}
		if (n0->right == NULL)
		{
			rDepth = 0;
		}
		else
		{
			rDepth = n0->right->depth;
		}
		if (lDepth > rDepth + 1)
		{
			n1 = n0->left;
			n2 = n1->right;
			n0->left = n2;
			n1->right = n0;
			n0->depth = NDepth(n0);
			*tree = n1;
		}
		else if (rDepth > lDepth + 1)
		{
			n1 = n0->right;
			n2 = n1->left;
			n0->right = n2;
			n1->left = n0;
			n0->depth = NDepth(n0);
			*tree = n1;
		}
		(*tree)->depth = NDepth(*tree);
	}
	return (ret);
}

TREE InsertNodeSingle(TREE *tree, void *key, void *info, TreeCmp keyCmp)
{
	return (InsertNodeSingle2(tree, key, info, keyCmp, NULL, NULL));
}

static TREE InsertNodeDupl2(TREE *root, void *key, void *info, int (*keyCmp)(void *, void *), TREE previous, TREE next)
{
	int lDepth;
	int rDepth;
	TREE n0 = *root;
	TREE n1;
	TREE n2;
	int res;
	TREE ret;
	
	if (n0 == NULL)
	{
		if (oldNodes == NULL)
		{
			n0 = (TREE) mmalloc(sizeof(struct tree));
			if (n0 == NULL)
			{
				perror("AVL Insert");
				exit(1);
			}
		}
		else
		{
			n0 = oldNodes;
			oldNodes = oldNodes->left;
		}
		n0->key = key;
		n0->info = info;
		n0->left = n0->right = NULL;
		n0->depth = 1;
		if (next != NULL)
		{
			n0->previous = next->previous;
			if (next->previous != NULL)
			{
				next->previous->next = n0;
			}
			next->previous = n0;
			n0->next = next;
		}
		else if (previous != NULL)
		{
			n0->next = previous->next;
			if (previous->next != NULL)
			{
				previous->next->previous = n0;
			}
			previous->next = n0;
			n0->previous = previous;
		}
		else
		{
			n0->next = NULL;
			n0->previous = NULL;
		}
		*root = n0;
		ret = n0;
	}
	else
	{
		if ((res = (*keyCmp)(n0->key, key)) > 0)
		{
			ret = InsertNodeDupl2(&(n0->left), key, info, keyCmp, NULL, n0);
		}
		else
		{
			ret = InsertNodeDupl2(&(n0->right), key, info, keyCmp, n0, NULL);
		}
		if (n0->left == NULL)
		{
			lDepth = 0;
		}
		else
		{
			lDepth = n0->left->depth;
		}
		if (n0->right == NULL)
		{
			rDepth = 0;
		}
		else
		{
			rDepth = n0->right->depth;
		}
		if (lDepth > rDepth + 1)
		{
			n1 = n0->left;
			n2 = n1->right;
			n0->left = n2;
			n1->right = n0;
			n0->depth = NDepth(n0);
			*root = n1;
		}
		else if (rDepth > lDepth + 1)
		{
			n1 = n0->right;
			n2 = n1->left;
			n0->right = n2;
			n1->left = n0;
			n0->depth = NDepth(n0);
			*root = n1;
		}
		(*root)->depth = NDepth(*root);
	}
	return (ret);
}

TREE InsertNodeDupl(TREE *tree, void *key, void *info, TreeCmp keyCmp)
{
	return (InsertNodeDupl2(tree, key, info, keyCmp, NULL, NULL));
}

TREE GetFirstNode(TREE tree)
{
	if (tree != NULL)
	{
		while (tree->left != NULL)
		{
			tree = tree->left;
		}
	}
	return (tree);
}

TREE GetLastNode(TREE tree)
{
	if (tree != NULL)
	{
		while (tree->right != NULL)
		{
			tree = tree->right;
		}
	}
	return (tree);
}

void SetInfo(TREE tree, void *info)
{
	tree->info = info;
}

void *GetInfo(TREE tree)
{
	return (tree == NULL? NULL: tree->info);
}

void *GetKey(TREE tree)
{
	return (tree == NULL? NULL: tree->key);
}

TREE GetNextNode(TREE tree)
{
	return (tree == NULL? NULL: tree->next);
}

TREE GetPreviousNode(TREE tree)
{
	return (tree == NULL? NULL: tree->previous);
}

void TraverseTreeInOrder(tree, proc)
TREE tree;
void (*proc)(void *, void *);
{
	if (tree != NULL)
	{
		TraverseTreeInOrder(tree->left, proc);
		(*proc)(tree->key, tree->info);
		TraverseTreeInOrder(tree->right, proc);
	}
}

void TraverseTreeInOrder2(tree, proc, info)
TREE tree;
void (*proc)(void *, void *, void *);
void *info;
{
	if (tree != NULL)
	{
		TraverseTreeInOrder2(tree->left, proc, info);
		(*proc)(tree->key, tree->info, info);
		TraverseTreeInOrder2(tree->right, proc, info);
	}
}

TREE FindNode(TREE tree, void *key, TreeCmp keyCmp)
{
	TREE n = tree;
	Boolean gevonden = false;
	int res;
	
	while (n != NULL && !gevonden)
	{
		if ((res = (*keyCmp)(n->key, key)) == 0)
		{
			gevonden = true;
		}
		else if (res < 0)
		{
			n = n->right;
		}
		else
		{
			n = n->left;
		}
	}
	return (n);
}

TREE FindApprox(TREE tree, void *key, TreeCmp keyCmp)
{
	TREE laatste = NULL;
	TREE n = tree;
	Boolean gevonden = false;
	int res;
	
	while (n != NULL && !gevonden)
	{
		if ((res = (*keyCmp)(n->key, key)) == 0)
		{
			gevonden = true;
		}
		else if (res < 0)
		{
			n = n->right;
		}
		else
		{
			laatste = n;
			n = n->left;
		}
	}
	return (gevonden? n: laatste);
}

void KillTree(TREE tree, void (*proc)(void *, void *))
{
	if (tree != NULL)
	{
		KillTree(tree->left, proc);
		if (proc != NULL)
		{
			(*proc)(tree->key, tree->info);
		}
		KillTree(tree->right, proc);
		tree->left = oldNodes;
		oldNodes = tree;
	}
}

int NrNodes(TREE tree)
{
	if (tree != NULL)
	{
		return (NrNodes(tree->left) + NrNodes(tree->right) + 1);
	}
	return (0);
}

long int CountTree(TREE tree, long int (*NodeCount)(void *, void *))
{
	if (tree == NULL) return (0);
	return (CountTree(tree->left, NodeCount) +
			CountTree(tree->right, NodeCount) +
			(NodeCount == NULL? 1: (*NodeCount)(tree->key, tree->info)));
}

static TREE Reorder(TREE node)
{
	short int lDepth = (node->left == 0? 0: node->left->depth);
	short int rDepth = (node->right == 0? 0: node->right->depth);
	TREE n1;
	TREE n2;

	if (lDepth > rDepth + 1)
	{
		n1 = node->left;
		n2 = n1->right;
		if (n2 == 0 || n2->depth < n1->depth - 1)
		{
			node->left = n2;
			n1->right = node;
			node->depth = NDepth(node);
			n1->depth = NDepth(n1);
			return (n1);
		}
		else
		{
			node->left = n2->right;
			n1->right = n2->left;
			n2->right = node;
			n2->left = n1;
			node->depth = NDepth(node);
			n1->depth = NDepth(n1);
			n2->depth = NDepth(n2);
			return (n2);
		}
	}
	else if (rDepth > lDepth + 1)
	{
		n1 = node->right;
		n2 = n1->left;
		if (n2 == 0 || n2->depth < n1->depth - 1)
		{
			node->right = n2;
			n1->left = node;
			node->depth = NDepth(node);
			n1->depth = NDepth(n1);
			return (n1);
		}
		else
		{
			node->right = n2->left;
			n1->left = n2->right;
			n2->left = node;
			n2->right = n1;
			node->depth = NDepth(node);
			n1->depth = NDepth(n1);
			n2->depth = NDepth(n2);
			return (n2);
		}
	}
	node->depth = (lDepth > rDepth? lDepth: rDepth) + 1;
	return (node);
}

static TREE delmin(TREE node, TREE *subst)
{
	if (node->left != NULL)
	{
		node->left = delmin(node->left, subst);
		return (Reorder(node));
	}
	else
	{
		*subst = node;
		return (node->right);
	}
}

static TREE delmax(TREE node, TREE *subst)
{
	if (node->right != NULL)
	{
		node->right = delmax(node->right, subst);
		return (Reorder(node));
	}
	else
	{
		*subst = node;
		return (node->left);
	}
}

static TREE RemoveNode(TREE node, void *key, int (*keyCmp)(void *, void *), void (*freeProc)(void *, void *))
{
	short int lDepth;
	short int rDepth;
	int rescmp;

	if (node == NULL)
	{
		return (NULL);
	}
	if ((rescmp = (*keyCmp)(node->key, key)) == 0)
	{
		TREE sNode = NULL;

		if (node->previous != NULL)
		{
			node->previous->next = node->next;
		}
		if (node->next != NULL)
		{
			node->next->previous = node->previous;
		}
		if (node->left == NULL && node->right == NULL)
		{
			if (freeProc != NULL) (*freeProc)(node->key, node->info);
			node->left = oldNodes; oldNodes = node;
			return (NULL);
		}
		lDepth = (node->left == 0? 0: node->left->depth);
		rDepth = (node->right == 0? 0: node->right->depth);
		if (lDepth > rDepth)
		{
			node->left = delmax(node->left, &sNode);
		}
		else
		{
			node->right = delmin(node->right, &sNode);
		}
		sNode->left = node->left;
		sNode->right = node->right;
		sNode->depth = NDepth(sNode);
		if (freeProc != NULL) (*freeProc)(node->key, node->info);
		node->left = oldNodes; oldNodes = node;
		return (Reorder(sNode));
	}
	else
	{
		if (rescmp > 0)
		{
			node->left = RemoveNode(node->left, key, keyCmp, freeProc);
		}
		else
		{
			node->right = RemoveNode(node->right, key, keyCmp, freeProc);
		}
		return (Reorder(node));
	}
}

void DeleteNode(TREE *tree, void *key, int (*keyCmp)(void *, void *), void (*nodeProc)(void *, void *))
{
	*tree = RemoveNode(*tree, key, keyCmp, nodeProc);
}

static TREE CopyTree2(TREE tree, CopyProc copyKey, CopyProc copyInfo)
{
	TREE n0;

	if (tree == NULL)
	{
		return (NULL);
	}
	if (oldNodes == NULL)
	{
		n0 = (TREE) mmalloc(sizeof(struct tree));
		if (n0 == NULL)
		{
			perror("CopyTree");
			exit(1);
		}
	}
	else
	{
		n0 = oldNodes;
		oldNodes = oldNodes->left;
	}
	n0->key = (copyKey == NULL? tree->key: copyKey(tree->key));
	n0->info = (copyInfo == NULL? tree->info: copyInfo(tree->info));
	n0->left = CopyTree(tree->left, copyKey, copyInfo);
	n0->right = CopyTree(tree->right, copyKey, copyInfo);
	n0->depth = tree->depth;
	return (n0);
}

static TREE SetOrder(TREE tree, TREE previous)
{
	if (tree == NULL)
	{
		return (previous);
	}
	tree->previous = SetOrder(tree->left, previous);
	if (tree->previous != NULL) tree->previous->next = tree;
	return (SetOrder(tree->right, tree));
}

TREE CopyTree(TREE tree, CopyProc copyKey, CopyProc copyInfo)
{
	TREE copy = CopyTree2(tree, copyKey, copyInfo);
	TREE finalNode = SetOrder(copy, NULL);

	if (finalNode != NULL) finalNode->next = NULL;
	return (copy);
}
