#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<conio.h>
#include<string.h>
#include<ctype.h>
#include<process.h>
#include<math.h>
#include<time.h>
#define N 100//upper bound to number of people
#define test 0//trying something out.

typedef struct temp {
	int ID;
	char name[30];
	int lifespan;//if -1, not dead yet.
	int age_of_parent;//age of parent when child was born
	struct temp **child;//dynamic array
	int children_limit;//for the dynamic array
	int number_of_children;//for the dynamic array
	//Additional Overheads
	struct temp *left;//needed to speed up random lookup with binary tree
	struct temp *right;//needed to speed up random lookup with binary tree
	struct temp *next;//needed for hashing
}person;

typedef struct {
	person **table;
	int size;
	int prime;
}hash;


void insert_tree(person **root, person p);
void destroy_tree(person *root);
void display_tree(person *root);
person* search_tree(person *b, char *s);//returns the person you want when you are in get_input.
void set_id(person *root);
void print_details(person *p);//prints a person's details
int number_of_people(person *root);//returns total number of people 
void print_generations(person *root);
void list_descendant(person *root, int k);//prints list of descendants of order k.
void link_people(person **dad, person *boy);
int live_descendant(person *root);//returns number of people who live to see their descendent of order k.
int get_descendants(int *x, person *p);//returns number of descendents of the person.
person* get_input(FILE *f);//returns get_original_ancestor.
person* get_original_ancestor(person *b);//returns original ancestor.
void set_hash(hash *h, person *p);
int hash_function(hash h, char *name);
void insert_hash(hash *h, person *p);
person* search_hash(hash h, char *s);
void destroy_hash(hash *h);

void main() {
	person *original;
	person *tree;
	hash h;
	char name[30];
	int *descendants, total, choice = 0;
	FILE *ifptr = fopen("input3.dat", "r");
	if (ifptr == NULL) {
		printf("Input file named \"input3.dat\" cannot be found.");
		exit(0);
	}
	tree = get_input(ifptr);//returns top of binary tree.
	original = get_original_ancestor(tree);
	set_hash(&h, tree);
	set_id(original);//As it wasn't given in input file, I'm using it to uniquely identify each person
	total = number_of_people(original);
	descendants = (int*)calloc(total, sizeof(int));
	for (int i = 0; i < total; i++) {
		descendants[i] = 0;
	}
	printf("\n\nUse only LOWER CASE...\n\n");
	while (1) {

		printf("\n\n0.Exit\n1.Name of original ancestor\n2.Number of descendants\n3.Person's details");
		printf("\n4.Number of people who see their great grandchildren\n5.List of a person's great grandchildren\n");
		printf("Enter your choice:");
		scanf("%d", &choice);
		if (choice == 0) {
			break;
		}
		switch (choice) {
		case 1:
			printf("%s\n", original->name);
			break;
		case 2:
			printf("\nEnter the persons name:");
			scanf("%s", name);
			printf("\n Number of descendants: %d", get_descendants(descendants, search_hash(h, name)));//search_tree(tree, name)));
			break;
		case 3:
			printf("\nEnter the persons name:");
			scanf("%s", name);		
			print_details(search_hash(h, name));//(search_tree(tree, name));
			break;
		case 4:
			printf("\nTotal Number of people: %d\n", live_descendant(original));
			break;
		case 5:
			printf("\nEnter the persons name:");
			scanf("%s", name);
			list_descendant(search_hash(h, name), 3);//(search_tree(tree, name), 3);
			break;
		}
	}
	print_generations(original);
	destroy_tree(tree);
	destroy_hash(&h);
	fclose(ifptr);
}

person* get_input(FILE *f) {
	person *b = NULL;
	char ch, age[30], son[30];
	person p;
	int id = 0;
	//Reading persons from file to a tree.
	fscanf(f, "%c", &ch);
	while(ch == '#'){
		do {//this is done to skip over the sentences in the beginning fo the file
			fscanf(f, "%c", &ch);
		} while (ch != '\n');//go to next line
		fscanf(f, "%c", &ch);
	}
	fscanf(f, "%s%s", &p.name, &age);
	p.name[strlen(p.name) + 1] = '\0';
	for (int i = strlen(p.name); i > 0; i--) {
		p.name[i] = p.name[i - 1];
	}
	p.name[0] = ch;
	p.lifespan = isdigit(age[0]) ? atoi(age) : -1;
	p.age_of_parent = p.number_of_children = 0;
	p.ID = ++id;//because no values have been given for it.
	p.children_limit = 3;
	p.child = (person **)calloc(p.children_limit, sizeof(person*));
	insert_tree(&b, p);
	while (!feof(f)) {
		fscanf(f, "%s%s", &p.name, &age);
		if (strlen(age) > 2 && !isdigit(age[0]))	break;
		p.lifespan = isdigit(age[0]) ? atoi(age) : -1;
		p.age_of_parent = p.number_of_children = 0;
		p.ID = ++id;//because no values have been given for it.
		p.children_limit = 3;
		p.child = (person **)calloc(p.children_limit, sizeof(person*));
		insert_tree(&b, p);
	}

	//Now I'm creating the links.
	strcpy(son, age);
	fscanf(f, "%s", &age);
	person *dad = search_tree(b, p.name);
	person *boy = search_tree(b, son);
	if (dad == NULL || boy == NULL) {
		printf("Input file has mentioned a person which doesn't exist.");
		destroy_tree(b);//to free memory
		exit(0);
	}
	boy->age_of_parent = isdigit(age[0]) ? atoi(age) : -1;
	link_people(&dad, boy);
	while (!feof(f)) {
		fscanf(f, "%s%s%s", &p.name, &son, &age);
		dad = search_tree(b, p.name);
		boy = search_tree(b, son);
		if (dad == NULL || boy == NULL) {
			printf("Input file has mentioned a person which doesn't exist.");
			destroy_tree(b);//to free memory
			exit(0);
		}
		boy->age_of_parent = isdigit(age[0]) ? atoi(age) : -1;
		link_people(&dad, boy);
	}
	
	return b;

}

void display_tree(person *root) {
	if (root == NULL) return;
	display_tree(root->left);
	printf("\t\t%s %d", root->name, root->lifespan);
	display_tree(root->right);
}

void insert_tree(person **root, person p) {
	if (*root == NULL) {
		person *next_element = (person *)malloc(sizeof(person));
		*next_element = p;
		next_element->next = NULL;
		next_element->left = next_element->right = NULL;
		*root = next_element;
	}
	else {
		if (strcmp(p.name, (*root)->name) <= 0) {
			insert_tree(&(*root)->left, p);
		}
		else {
			insert_tree(&(*root)->right, p);
		}
	}
}

person* search_tree(person *b, char *s) {
	if (b == NULL)	return NULL;
	if (strcmp(s, b->name) == 0) {
		return b;
	}
	else if (strcmp(s, b->name) > 0) {
		return search_tree(b->right, s);
	}
	else {
		return search_tree(b->left, s);
	}
}

void link_people(person **dad, person *boy) {
	(*dad)->number_of_children++;
	if ((*dad)->number_of_children <= (*dad)->children_limit) {
		(*dad)->child[(*dad)->number_of_children - 1] = boy;
	}
	else {
		(*dad)->children_limit *= 2;
		person **new_set = (person**) calloc((*dad)->children_limit, sizeof(person*));
		for (int i = 0; i < (*dad)->children_limit / 2; i++) {
			new_set[i] = (*dad)->child[i];
		}
		new_set[(*dad)->number_of_children - 1] = boy;
		(*dad)->child = new_set;
	}
}

void destroy_tree(person *root) {//only for unexpected situations which require exits.
	if (root == NULL) return;
	destroy_tree((root)->left);
	destroy_tree((root)->right);
	free(root);
}

person* get_original_ancestor(person *b) {
	if (b == NULL)	return NULL;
	if (b->age_of_parent == 0)	return b;//age_of_parent = 0 => No parent.
	person *topL = get_original_ancestor(b->left);
	person *topR = get_original_ancestor(b->right);
	if (topL != NULL) {
		if (topR != NULL) {
			printf("\n\nMore than 1 original ancestor has been detected.\n\n");
			destroy_tree(b);
			exit(0);
		}
		return topL;
	}
	return topR;
}

void print_generations(person *root) {
	person *queue[N];//a simple circular queue
	person *next, *end = root, *end_candidate;
	unsigned int front = 0, rear = 0;
	FILE *ofptr = fopen("output3.dat", "w");
	fprintf(ofptr, "This is the geneology listed by generation.\n\n");
	queue[rear++] = root;
	while (front != rear && end != NULL) {
		next = queue[front++];
		front %= N;
		fprintf(ofptr, "%s\t", next->name);
		//fprintf(ofptr, "%d\t", next->ID);
		for (int i = 0; i < next->number_of_children; i++) {
			queue[rear++] = next->child[i];
			rear %= N;
		}
		if (next->number_of_children > 0) {
			end_candidate = next->child[next->number_of_children - 1];
		}
		if (next == end) {
			fprintf(ofptr, "\n");
			end = end_candidate;
			end_candidate = NULL;
		}
	}
	fclose(ofptr);
}

void set_id(person *root) {
	person *queue[N];//a simple circular queue
	person *next;
	unsigned int front = 0, rear = 0, id = 0;
	queue[rear++] = root;
	while (front != rear) {
		next = queue[front++];
		front %= N;
		next->ID = id++;
		for (int i = 0; i < next->number_of_children; i++) {
			queue[rear++] = next->child[i];
			rear %= N;
		}
	}
}

int number_of_people(person *root) {
	person *queue[N];//a simple circular queue
	person *next;
	unsigned int front = 0, rear = 0, number_of_people = 0;
	queue[rear++] = root;
	while (front != rear) {
		next = queue[front++];
		front %= N;
		number_of_people++;
		for (int i = 0; i < next->number_of_children; i++) {
			queue[rear++] = next->child[i];
			rear %= N;
		}
	}
	return number_of_people;
}

int get_descendants(int *x, person *p) {
	if (p == NULL)	return 0;
	if (x[p->ID] != 0 || p->number_of_children == 0) {
		return x[p->ID];
	}

	for (int i = 0; i < p->number_of_children; i++) {
		x[p->ID] += (get_descendants(x, p->child[i]) + 1);
	}
	return x[p->ID];
}

int live_descendant(person *root) {
	person *queue[N];//a simple circular queue
	person *next;
	unsigned int front = 0, rear = 0, number = 0;
	queue[rear++] = root;
	while (front != rear) {
		next = queue[front++];
		front %= N;
		//fprintf(ofptr, "%s\t", next->name);
		int z = 0;
		for (int i = 0; i < next->number_of_children && z==0; i++) {
			for (int j = 0; j < next->child[i]->number_of_children && z == 0; j++) {
				person *temp = next->child[i]->child[j];
				for (int k = 0; k < next->child[i]->child[j]->number_of_children && z == 0; k++) {
					z += (temp->child[k]->age_of_parent + temp->age_of_parent + next->child[i]->age_of_parent <= next->lifespan);
					if (z) {
						printf("%s\t", next->name);
					}
				}
			}
		}
		number += (z > 0);
		for (int i = 0; i < next->number_of_children; i++) {
			queue[rear++] = next->child[i];
			rear %= N;
		}
	}
	return number;
}

void list_descendant(person *root, int k) {
	person *queue[N];//a simple circular queue
	person *next, *end = root, *end_candidate;
	unsigned int front = 0, rear = 0, generation_gap = 0;
	queue[rear++] = root;
	printf("\n");
	while (front != rear && end != NULL && generation_gap <= k) {
		next = queue[front++];
		front %= N;
		if (generation_gap == k) {
			printf("%s\t", next->name);
		}
		for (int i = 0; i < next->number_of_children; i++) {
			queue[rear++] = next->child[i];
			rear %= N;
		}
		if (next->number_of_children > 0) {
			end_candidate = next->child[next->number_of_children - 1];
		}
		if (next == end) {
			end = end_candidate;
			end_candidate = NULL;
			generation_gap++;
		}
	}
}

void print_details(person *p) {
	if (p == NULL) {
		printf("\nNot available.\n");
		return;
	}
	printf("\nName: %s\t\tNumber of children: %d", p->name, p->number_of_children);
	char temp[20];
	if (p->lifespan > 0) {
		sprintf(temp, "%d", p->lifespan);
	}
	else {
		strcpy(temp, "Still alive!");
	}
	printf("\nAge of death: %s", temp);
	return;
}

int hash_function(hash h, char *name) {
	name = _strlwr(name);
	long sum = 0;
	for (int i = 0; i < strlen(name); i++) {
		sum += i*i*(name[i] - 'a');
	}
	return sum % h.prime;
}

void set_hash(hash *h, person *p) {//creates a hash table from the tree.
	h->size = number_of_people(p);
	int temp = 3 * h->size / 2 + (3 * h->size / 2) % 2 + 1;
	int i;
	h->prime = 0;
	while (h->prime == 0) {
		temp += 2;
		for (i = 3; i <= sqrt(temp); i += 2) {
			if (temp % i == 0) break;
		}
		if (i > sqrt(temp)) {
			h->prime = temp;
		}
	}
	h->table = (person**)calloc(h->prime, sizeof(person*));

	person *queue[N];//a simple circular queue
	person *next;
	unsigned int front = 0, rear = 0;
	queue[rear++] = p;
	while (front != rear) {
		next = queue[front++];
		front %= N;
		insert_hash(h, next);
		if (next->left != NULL) {
			queue[rear++] = next->left;
			rear %= N;
		}
		if (next->right != NULL) {
			queue[rear++] = next->right;
			rear %= N;
		}
	}
}

void insert_hash(hash *h, person *p) {
	int location = hash_function(*h, p->name);//h->table;
	person *x = h->table[location];
	//if ((h->table)[hash_function(*h, p->name)]);
	if (x == NULL) {
		h->table[location] = p;
	}
	else {
		while (x->next != NULL) {
			x = x->next;
		}
		x->next = p;
	}
}

person* search_hash(hash h, char *s) {
	int index = hash_function(h,s);
	person *x = h.table[index];
	if (x == NULL)	return NULL;
	while (x->next != NULL) {
		if (strcmp(x->name, s) == 0) {
			return x;
		}
		x = x->next;
	}
	if (strcmp(x->name, s) == 0) {
		return x;
	}
	return NULL;
}

void destroy_hash(hash *h) {
	free(h->table);
}
