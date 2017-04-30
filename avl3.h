/* TYPEDEFINITIES
 */

typedef struct tree *TREE;
typedef int (*TreeCmp)(void *, void *);
typedef void (*TreeProc)(void *, void *);
typedef void (*TreeProc2)(void *, void *, void *);
typedef void *(*CopyProc)(void *);

/* PROTOTYPES
 */

extern TREE InsertNodeDupl(TREE *tree, void *key, void *info, TreeCmp keyCmp);
extern TREE InsertNodeSingle(TREE *tree, void *key, void *info, TreeCmp keyCmp);
extern void TraverseTreeInOrder(TREE tree, TreeProc treeProc);
extern void TraverseTreeInOrder2(TREE tree, TreeProc2 treeProc2, void *);
extern void KillTree(TREE tree, TreeProc treeProc);
extern TREE FindNode(TREE tree, void *key, TreeCmp keyCmp);
extern TREE FindApprox(TREE tree, void *key, TreeCmp keyCmp);
extern void *GetKey(TREE tree);
extern void *GetInfo(TREE tree);
extern void SetInfo(TREE tree, void *info);
extern TREE CopyTree(TREE tree, CopyProc copyKey, CopyProc copyInfo);
extern TREE GetFirstNode(TREE tree);
extern TREE GetLastNode(TREE tree);
extern TREE GetNextNode(TREE tree);
extern TREE GetPreviousNode(TREE tree);
extern long int CountTree(TREE tree, long int (*treeCount)(void *, void *));
extern void DeleteNode(TREE *tree, void *key, TreeCmp keyCmp, TreeProc freeProc);
extern int NrNodes(TREE tree);
