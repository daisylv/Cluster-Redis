/*
 * cluster.c
 *
 *  Created on: Apr 18, 2014
 *      Author: daisy
 */
#include <stdarg.h>
#include <string.h>
#include "cluster.h"
#include "../libconhash/conhash.h"


void free_node_s_inlist(node_s_inlist *listnode) {
	if (listnode->childern) {
		free_node_s_inlist(listnode->childern);
	}
	if (listnode->next) {
		free_node_s_inlist(listnode->next);
	}
	if (listnode->childern) {
		free(listnode->childern->node);
		free(listnode->childern);
	}
	if (listnode->next) {
		free(listnode->next->node);
		free(listnode->next);
	}
	free(listnode->node);
	free(listnode);
}

cluster* initialcluster(char *name) {
	cluster *_cluster;
	if (!name)
		return NULL;
	_cluster = (cluster *) malloc(sizeof(cluster));
	_cluster->clustername = name;
	_cluster->conhash = conhash_init(NULL);
	_cluster->nodelisthead = NULL;

	return _cluster;
}

void clusteraddnode(cluster *_cluster, char *nodenamelist) {
	if (!_cluster)
		return;
	struct node_s_inlist *last, *newnode;
	last = _cluster->nodelisthead;
	while (last && last->next != NULL) {
		last = last->next;
	}
	char a[128] = "";
	char *ch = nodenamelist;
	int i = 0;
	while (*ch != '\0') {
		a[i++] = *ch;
		ch++;
	}
	char *nodename = strtok(a, " ");
	while (nodename != NULL) {
		newnode = (node_s_inlist*) malloc(sizeof(node_s_inlist));
		//struct node_s *node =
		newnode->node = (struct node_s*) malloc(sizeof(struct node_s));
		newnode->childern = NULL;
		newnode->conhash = NULL;
		newnode->next = NULL;
		newnode->node->pointer = newnode;
		struct node_s* node = newnode->node;
		conhash_set_node(node, nodename, 1000);
		conhash_add_node(_cluster->conhash, node);
		if (last == NULL) {
			_cluster->nodelisthead = newnode;
			last = newnode;
		} else {
			last->next = newnode;
			last = last->next;
		}
		nodename = strtok(NULL, " ");
	}
}

void removeclusternode(cluster *_cluster, char * nodenamelist) {
	if (!_cluster)
		return;
	char a[128] = "";
	char *ch = nodenamelist;
	int i = 0;
	while (*ch != '\0') {
		a[i++] = *ch;
		ch++;
	}
	char *nodename = strtok(a, ".");
	while (nodename != NULL) {
		node_s_inlist *cur = _cluster->nodelisthead;
		while (cur != NULL) {
			if (strcmp(cur->node->iden, nodename) == 0) {
				conhash_del_node(_cluster->conhash, cur->node);
				if (cur->next != NULL) {
					node_s_inlist *gc;
					gc = cur->next;
					cur->node = cur->next->node;
					cur->conhash = cur->next->conhash;
					cur->childern = cur->next->childern;
					cur->next = cur->next->next;
					free_node_s_inlist(gc);
				} else {
					free_node_s_inlist(cur);
				}
				break;
			}
			cur = cur->next;
		}
		nodename = strtok(NULL, ".");
	}
}

void addnodechild(cluster *_cluster, char *target, char *childnodenamelist) {
	if (_cluster == NULL)
		return;
	char a[128] = "";
	char *ch = childnodenamelist;
	int i = 0;
	while (*ch != '\0') {
		a[i++] = *ch;
		ch++;
	}
	char *nodename = strtok(a, ".");
	node_s_inlist *parent = NULL;
	node_s_inlist *cur = _cluster->nodelisthead;
	int flag = 0;
	while (nodename != NULL) {
		flag = 0;
		while (cur != NULL) {
			if (strcmp(cur->node->iden, nodename) == 0) {
				flag = 1;
				parent = cur;
				cur = cur->childern;
				break;
			} else {
				cur = cur->next;
			}
		}
		if (flag == 0)
			return;
		nodename = strtok(NULL, ".");
	}
	if (parent == NULL || flag == 0)
		return;
	if (parent->conhash == NULL)
		parent->conhash = conhash_init(NULL);
	node_s_inlist *newnode = (node_s_inlist*) malloc(sizeof(node_s_inlist));
	newnode->childern = NULL;
	newnode->conhash = NULL;
	newnode->next = NULL;
	newnode->node = (struct node_s*) malloc(sizeof(struct node_s));
	newnode->node->pointer = newnode;
	newnode->next = parent->childern;
	parent->childern = newnode;
	conhash_set_node(newnode->node, target, 1000);
	conhash_add_node(parent->conhash, newnode->node);
}

void delnodechild(cluster *_cluster, char *target, char *childnodenamelist) {
	if (!_cluster)
		return;
	char a[128] = "";
	char *ch = childnodenamelist;
	int i = 0;
	while (*ch != '\0') {
		a[i++] = *ch;
		ch++;
	}
	node_s_inlist *parent;
	node_s_inlist *cur = _cluster->nodelisthead;
	int flag = 0;
	char *nodename = strtok(a, ".");
	while (nodename != NULL) {
		flag = 0;
		while (cur != NULL) {
			if (strcmp(cur->node->iden, nodename) == 0) {
				flag = 1;
				parent = cur;
				cur = cur->childern;
				break;
			} else {
				cur = cur->next;
			}
		}
		if (flag == 0)
			return;
		nodename = strtok(NULL, ".");
	}
	if (flag == 0)
		return;
	flag = 0;
	while (cur != NULL) {
		if (strcmp(cur->node->iden, target) == 0) {
			flag = 1;
			break;
		}
		cur = cur->next;
	}
	if (flag != 0) {
		conhash_del_node(parent->conhash, cur->node);
		if (cur->next != NULL) {
			node_s_inlist *gc;
			gc = cur->next;
			cur->node = cur->next->node;
			cur->conhash = cur->next->conhash;
			cur->childern = cur->next->childern;
			cur->next = cur->next->next;
			free_node_s_inlist(gc);
		} else {
			free_node_s_inlist(cur);
		}
	}
}

const char* getserver(cluster *_cluster, char *key) {
	if (!_cluster || !_cluster->nodelisthead) {
		//print error
		return NULL;
	}
	const struct node_s *cur = conhash_lookup(_cluster->conhash, key);
	node_s_inlist *pa = (node_s_inlist*)cur->pointer;
	while (pa->childern) {
		if (pa->conhash) {
			cur = conhash_lookup(pa->conhash, key);
			pa = (node_s_inlist*)cur->pointer;
		}
		else {
			// error printf
			return NULL;
		}
	}
	//cur = conhash_lookup(pa->conhash, key);
	return cur->iden;
}
