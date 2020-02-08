#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<assert.h>

// strings.h is an extension of string.h with some less standardized functions

/*
 * arrange.c
 * author: Francisco Belliard
 * date: Feb 5, 16:13
 * last update: 
 *
 */

#define USAGE_MESSAGE "usage: arrange [-v] [-r _to_remove_] _to_add_ "
#define DELIMIT ":"

int verbose_g = 0 ;

struct Node {
	char * word ;
	struct Node * next ;
} ;

struct Node * new_ll( void ) {
	// return a new node to be the list anchor
	struct Node * node = (struct Node *) malloc(sizeof(struct Node)) ;
	memset(node, 0, sizeof(struct Node)) ;
	return node ;
}

void add_ll( struct Node * anchor, char * word ) {
	// given a pointer to the anchor of the list, and a word, add the word
	// to the list, does not matter where (except don't replace the anchor)
  struct Node * n = (struct Node *)malloc(sizeof(struct Node));
  if (n == NULL) {fprintf(stderr, "Error, exiting\n");}
  n->word = word; //assuming the word is already stored somewhere that it won't
                  //get deleted
  n->next = anchor->next; //point this node's next at the anchor's current next
  anchor->next = n; //right now, anchor and n are pointing at same thing
                    //so point anchor next at the new node and voila
	// ...

	return ;  
}

struct Node * find_ll(struct Node * anchor, char * word) {
	// given a pointer to the anchor of the list, and a word, search
	// the list for the word. return the pointer to the with the word, if found,
	// or NULL if not found
	struct Node * n = anchor;

	if (n == NULL) //check if list is empty, it should have just the anchor pointer
	{			   //but we want to be sure that before we dereference the pointer that it exists.
	  fprintf(stderr, "Error, empty list\n");
	} 
	else //otherwise move pointer to first node in list (remember anchor pointer is position 0)
	{
		n = n->next; 
	}

 	while (n != NULL) //if the list isn't empty, traverse list
	{
		if (strcmp(n->word, word) == 0) //if the word in list == word we are searching for return the node at that point
		{
			return n;
		} 
		else //otherwise, keep traversing list
		{
			n = n->next;
		}
	 }

	//if here, word wasn't found in list
	fprintf(stderr, "%s not found in list\n", word);
	return NULL; //returns null if not found

}


void remove_ll( struct Node * anchor, struct Node * node_to_remove ) {
	// given a list anchor, and a pointer to a node in the list (but not the anchor)
	// do pointer surgery to remove the node, and free it.

  struct Node * n = anchor; //temp pointer used to traverse list, set it to the list anchor
  if(n == NULL){return;} //checks the anchor isn't null

  while(n->next != NULL) //traverse the list
  {
	if(n->next == node_to_remove) //if the node to remove is found exit loop
	{
		break;
	}
	else
	{
		n = n->next; //otherwise continue searching list
	}
  } //end of loop, if node is found then it will be in n->next

  if(n == NULL) //checks if element is found in the list
  {
	  fprintf(stderr,"Node not in list");
  }
  else //otherwise, node was found and is n->next
  {
	  n->next = node_to_remove->next; 
	  node_to_remove->next = NULL;
	  free(node_to_remove);
	  node_to_remove = NULL;
  }
// 	// sanity checks
// 	//assert(node_to_remove); 
}

void print_ll( struct Node *anchor ) {
	struct Node * node ;
	node = anchor->next ;
	printf("list-> ") ;
	if (!node) printf("-- list is empty --") ;
	while (node) {
		printf("|%s| ",node->word) ;
		node = node->next ;
	}
	printf("\n") ;
	return ;
}

int main(int argc, char * argv[]) {
	int ch ;
	char * word ;
	struct Node * list_anchor ;
	char * remove_these = "" ;
	char * add_these ;
	
	while ((ch = getopt(argc, argv, "vr:")) != -1) {
		switch(ch) {
		case 'v':
			verbose_g++ ;
			break ;
		case 'r':
			remove_these = strdup(optarg) ;
			break ;
		default:
			printf("%s\n", USAGE_MESSAGE) ;
			return 0 ;
		}
	}
	argc -= optind; // these are globals defined in the getopt code
	argv += optind;

	if (argc!=1) {
		printf("%s\n", USAGE_MESSAGE) ;
		return 0 ;
	}
	add_these = strdup(argv[0]) ;
	
	if (verbose_g>1) {
		printf("verbose: %d\nto remove: |%s|\nto add: |%s|\n", verbose_g, remove_these, add_these) ;
	}
	
	list_anchor = new_ll() ;
	
	// insertions to process
	word = strtok(add_these,DELIMIT) ;
	while (word) {
		add_ll(list_anchor,word) ;
		if (verbose_g>0) print_ll(list_anchor) ;
		word = strtok(NULL,DELIMIT) ;
	}
	print_ll(list_anchor) ;
	
	// deletions to process
	if (strlen(remove_these)>0) {
		struct Node * n ;
		word = strtok(remove_these,DELIMIT) ;
		while (word) {	
			n = find_ll(list_anchor,word) ;
			if (n) remove_ll(list_anchor,n) ;
			if (verbose_g>0) print_ll(list_anchor) ;
			word = strtok(NULL,DELIMIT) ;
		}
		print_ll(list_anchor) ;
	}
	

	return 0 ;
}

