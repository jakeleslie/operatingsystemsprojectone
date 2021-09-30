#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>


// Struct definition of the BST Node.
// The root node can be passed around as a reference to the entire structure.
// Similarly, a node can be thought of as a reference to the subtree!
// Null pointers refer to the empty tree
struct bst_node {
  pthread_mutex_t mutex;  // Mutex for this subtree. If this is held, all children should not be modified by another thread.
  int key_value;          // Value stored in the root node of this subtree.
  struct bst_node *left;  // Left child. left->key_value <= key_value
  struct bst_node *right; // Right child. right->key_value >= key_value
};

// Free memory associated with this node and recursively call on its children
void bst_destroy(struct bst_node *leaf) {
  if (leaf != NULL) {
    bst_destroy(leaf->left);
    bst_destroy(leaf->right);
    free(leaf);
  }
}

// Create a new node with this key in the subtree pointed to by leaf.
void bst_insert(int key, struct bst_node **leaf) {
  // If this is an empty tree, create a new node and
  // replece the empty tree leaf points to with that new leaf node.
  if (*leaf == NULL) {
    *leaf = (struct bst_node *)malloc(sizeof(struct bst_node));
    pthread_mutex_init(&(*leaf)->mutex, NULL);
    (*leaf)->key_value = key;
    // Children are initially empty trees
    (*leaf)->left = NULL;
    (*leaf)->right = NULL;
  } else if (key < (*leaf)->key_value) { // Traverse leftward if the value is smaller than the root's
    bst_insert(key, &(*leaf)->left);
  } else if (key > (*leaf)->key_value) { // Traverse rightward if the value is larger than the root's
    bst_insert(key, &(*leaf)->right);
  }
}

// Find a given key in the tree, returning a pointer to the node containing that key.
// If no such key exists, return an empty tree, aka NULL
struct bst_node *bst_search(int key, struct bst_node *root) {
  if (root != NULL) {
    if (key == root->key_value) {
      return root;
    } else if (key < root->key_value) {
      return bst_search(key, root->left);
    } else {
      return bst_search(key, root->right);
    }
  } else
    return NULL;
}

// Helper function to validate a tree:
//  A for a tree to be "helper-valid", the tree's nodes must be sorted
//    when traversed in-order and the values must be bounded by the arguments min and max.
//  If the tree is helper-invalid, the function will return -1
//  If the tree is helper-valid, the function will return the number of nodes in the tree.
int bst_validate_helper(struct bst_node *leaf, int min, int max) {
  // An empty BST is valid and has no elements.
  if (leaf == NULL) {
    return 0;
  }
  // If this node is not in [min, max], return invalidity
  if (leaf->key_value < min || leaf->key_value > max) {
    return -1;
  }
  // Check if the child subtrees are valid.
  // We must bound by the current node's value, in order to narrow down the range as we go down the tree.
  int left_count = bst_validate_helper(leaf->left, min, leaf->key_value);
  int right_count = bst_validate_helper(leaf->right, leaf->key_value, max);

  // If either child subtree was invalid, this tree is invalid
  if (left_count == -1 || right_count == -1) {
    return -1;
  }

  // We have a valid subtree!
  // Return the size of the current subtree, including this node
  return 1 + left_count + right_count;
}

// Validate a tree
// Returns -1 for an invalid tree and the number of nodes for a valid tree.
// Calls helper with initial values.
int bst_validate(struct bst_node *leaf) {
  return bst_validate_helper(leaf, INT_MIN, INT_MAX);
}


// Parallel worker to insert nodes.
//    ptr is an array of pointers to the parameters of the function, parsed in the first few lines of the program.
// This function inserts the elements [min, max) of arr into the tree specified by root.
// It must do so in a way that is thread-safe. Meaning, that multiple instances of this function will run at the same time.
void *parallel_insert_worker(void *ptr) {
  // Parse the arguments
  void **ptr_arr = (void **)ptr;
  int min = *((int *)ptr_arr[0]);
  int max = *((int *)ptr_arr[1]);
  int *arr = (int *)ptr_arr[2];
  struct bst_node **root = *(struct bst_node ***)ptr_arr[3];

  // Debugging output to make sure we have the arguments passed correctly.
  printf("[DEBUG] Inserting bulk inputs number %d - %d from array %p into tree %p\n", min, max, arr, *root);

  // TODO: Fix the code below.
  for (int i = min; i < max; i++) {

    struct bst_node **leaf = root;
    int key = arr[i];

    pthread_mutex_lock(&(*root)->mutex); //added this, it locks the entire tree but we do not want this we want to lock specific threads, but this will
    
    pthread_mutex_t* parent = (&(*root)->mutex);

    while(1) {
      printf("%d\n", *leaf);
      if (*leaf == NULL) { //if null run this to add values to it 
        *leaf = (struct bst_node *)malloc(sizeof(struct bst_node));
        pthread_mutex_init(&(*leaf)->mutex, NULL); //code should go under here, because this creates our leaf mutex 
        (*leaf)->key_value = key;
        // Children are initially empty trees
        (*leaf)->left = NULL;
        (*leaf)->right = NULL;
        pthread_mutex_unlock(parent);
        break;
      } else if (key < (*leaf)->key_value) { // Traverse leftward if the value is smaller than the root's
       if((*leaf)->left != NULL){
        pthread_mutex_lock(&(*leaf)->left->mutex); //add
        leaf =  &(*leaf)->left;
        pthread_mutex_unlock(parent);
        parent = (&(*leaf)->mutex);
       }
       else{
         //value pointed to by leaf = NULL
         *leaf = NULL;
       }

      } else if (key > (*leaf)->key_value) { // Traverse rightward if the value is larger than the root's
        
        if((*leaf)->right != NULL){
        pthread_mutex_lock(&(*leaf)->right->mutex); //addition
        leaf =  &(*leaf)->right;
        pthread_mutex_unlock(parent);
        parent = (&(*leaf)->mutex);
        }
        else{
          *leaf = NULL;
        }
      }//hold the lock as you enter 
      
    }
  //keep track of parent cna use ptr to mutex form, and then unlock it. points to the lock at the beginning of the else 
  //update parent, at the correct time and check what it is 
  }

  return NULL;
}

// Insert n_keys elements into the tree pointed to by root, using n_threads simultaneous threads.
void bst_bulk_insert(int n_keys, int *keys, struct bst_node **root,
                     int n_threads) {
  // Assume a sane number of threads
  assert(n_threads < 4096);
  // Allocate the array of argument array structures to pass to the threads
  void ***args = malloc(n_threads * sizeof(void **));
  int *min_values = malloc(n_threads * sizeof(int *));
  int *max_values = malloc(n_threads * sizeof(int *));
  // Create the thread structures
  pthread_t *thread_structs = malloc(n_threads * sizeof(pthread_t *));

  // For each thread we want to run.
  for (int i = 0; i < n_threads; i++) {
    // Initialize the bounds this thread should insert.
    min_values[i] = (i * n_keys)/n_threads;
    if (i < n_threads - 1) {
      max_values[i] = ((i+1) * n_keys)/n_threads;
    } else {
      max_values[i] = n_keys;
    }
    // alloc the argument array structure for this thread
    args[i] = malloc(4 * sizeof(void *));
    // assign the arguments for this thread
    args[i][0] = (void *)&min_values[i];
    args[i][1] = (void *)&max_values[i];
    args[i][2] = (void *)keys;
    args[i][3] = (void *)&root;
    // Create this thread and pass the arguments.
    pthread_create(&thread_structs[i], NULL, *parallel_insert_worker, args[i]);
  }

  // Wait for each thread to finish and deallocate the argument structures and thread structures.
  for (int i = 0; i < n_threads; i++) {
    pthread_join(thread_structs[i], NULL);
    free(args[i]);
  }
  free(args);
  free(min_values);
  free(max_values);
  free(thread_structs);
  return;
}

int main() {

  // Declare variables
  int count = 1000000;
  int *arr = malloc(sizeof(int) * count);
  struct timeval stop, start;
  struct bst_node *root = NULL;

  // Test 1
  printf("Performing simple insert test:\n");
  srand(48221);
  for (int i = 0; i < count; i++) {
    bst_insert(rand(), &root);
  }
  assert(bst_validate(root) == count);
  printf("\t PASS\n");

  // Test 2
  printf("Performing simple insert performance test:\n");
  for (int i = 0; i < count; i++) {
    arr[i] = rand();
  }
  gettimeofday(&start, NULL);
  for (int i = 0; i < count; i++) {
    bst_insert(arr[i], &root);
  }
  gettimeofday(&stop, NULL);
  assert(bst_validate(root) == 2*count);
  printf("DONE: %ld milliseconds\n", ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec)/1000);

  // Test 3
  printf("Performing deletion test:\n");
  bst_destroy(root);
  root = NULL;
  printf("\t PASS\n");

  // Test 4
  printf("Performing bulk insert test:\n");
  srand(48221);
  for (int i = 0; i < count; i++) {
    arr[i] = rand();
  }
  bst_insert(arr[0], &root);
  bst_bulk_insert(count-1, arr+1, &root, 8);
  assert(bst_validate(root) == count);
  printf("\t PASS\n");

  // Test 5
  printf("Performing bulk insert performance test:\n");
  for (int i = 0; i < count; i++) {
    arr[i] = rand();
  }
  gettimeofday(&start, NULL);
  bst_bulk_insert(count, arr, &root, 8);
  gettimeofday(&stop, NULL);
  assert(bst_validate(root) == 2*count);
  printf("DONE: %ld milliseconds\n", ((stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec)/1000);

  printf("Inserted %d elements total.\n", bst_validate(root));

  assert(bst_validate(root) == count*2);
  printf("\t PASS\n");

  return 0;
}
