/*
 * cluster.c
 *
 *  Created on: Apr 18, 2014
 *      Author: daisy
 */
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "cluster.h"
#include "../libconhash/conhash.h"


void free_node_s_inlist(node_s_inlist *listnode) {
	if (listnode->childern) {
		free_node_s_inlist(listnode->childern);
	}
	if (listnode->next) {
		free_node_s_inlist(listnode->next);
	}
	if (listnode->childern && listnode->childern->node) {
		free(listnode->childern->node);
		free(listnode->childern);
	}
	if (listnode->next && listnode->next->node) {
		free(listnode->next->node);
		free(listnode->next);
	}
	if (listnode->childern)
		free(listnode->childern);
	if (listnode->node) {
		free(listnode->node);
		listnode->node = NULL;
	}
}

cluster* initialcluster(char *name) {
	cluster *_cluster;
	if (!name)
		return NULL;
	_cluster = (cluster *) malloc(sizeof(cluster));
	strcpy(_cluster->clustername, name);
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
		node_s_inlist *parent;
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

				cur = cur->next;
				parent->next = cur;
				break;
			}
			parent = cur;
			cur = cur->next;
			parent->next = cur;
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
		if (parent->childern->node == NULL) {
			removeclusternode(_cluster, parent->node->iden);
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

clusterlist * loadClusterData(char *filename) {
	clusterlist *_clusterlist = NULL;
	clusterlist *_clusterlisttail = NULL;
	FILE *fd = fopen(filename, "r");
	if (fd == NULL){
		printf("error when loading cluster data...");
		return NULL;
	}
	char buffer[256] = "";
	fgets(buffer, 256, fd);
	int num = atoi(buffer);
	if (num == 0)
		return NULL;
	int i;
	for(i = 0; i < num; ++i) {
		fgets(buffer, 256, fd);
		if (strcmp(buffer, "start\n") != 0) {
			continue;
		}
		if (strcmp(buffer, "end\n") == 0)
			continue;
		fgets(buffer, 256, fd);
		int len = strlen(buffer);
		buffer[len-1] = '\0';
		char clustername[64];
		strcpy(clustername, buffer);
		cluster *_cluster = initialcluster(clustername);

		fgets(buffer, 256, fd);
		len = strlen(buffer);
		buffer[len-1] = '\0';
		char groups[128];
		strcpy(groups, buffer);
		clusteraddnode(_cluster, groups);
		while (fgets(buffer, 256, fd) != NULL) {
			len = strlen(buffer);
			buffer[len-1] = '\0';
			char nodeinfo[128] = "";
			strcpy(nodeinfo, buffer);
			char *parent = strtok(nodeinfo, " ");
			char *child = strtok(NULL, " ");
			while (child != NULL) {
				addnodechild(_cluster, child, parent);
				child = strtok(NULL, " ");
			}
		}
		if(_clusterlist == NULL) {
			_clusterlist = (clusterlist*)malloc(sizeof(clusterlist));
			_clusterlist->_cluster = _cluster;
			_clusterlist->next = NULL;
			_clusterlisttail = _clusterlist;
		}
		else {
			clusterlist *newcluster = (clusterlist*)malloc(sizeof(clusterlist));
			newcluster->_cluster = _cluster;
			newcluster->next = NULL;
			_clusterlisttail->next = newcluster;
			_clusterlisttail = _clusterlisttail->next;
		}
	}
	printf("cluster loaded...\n");
	printf("%s\n", _clusterlist->_cluster->clustername);
	return _clusterlist;
}

void writechildnodes(FILE *file, node_s_inlist *node_in_list, char *path) {
	if(path != NULL) {
		fwrite(path, 1, strlen(path), file);
		fwrite(" ", 1, 1, file);
		fwrite(node_in_list->node->iden, 1, strlen(node_in_list->node->iden), file);
		fwrite("\n", 1, 1, file);
	}
	if(node_in_list->next != NULL) {
		writechildnodes(file, node_in_list->next, path);
	}

	if(node_in_list->childern != NULL) {
		char full[256] = "";
		strcat(full, path);
		if(*full != '\0') {
			strcat(full, ".");
			strcat(full, node_in_list->node->iden);
			writechildnodes(file, node_in_list->childern, full);
	     	} else {
			strcat(full, node_in_list->node->iden);
			writechildnodes(file, node_in_list->childern, full);
		}
	}
}

void writenodes(FILE *file, cluster *_cluster) {
	node_s_inlist *nodes = _cluster->nodelisthead;
	while(nodes->next != NULL) {
		fwrite(nodes->node->iden, 1, strlen(nodes->node->iden), file);
		fwrite(" ", 1, 1, file);
		nodes = nodes->next;
	}
	fwrite(nodes->node->iden, 1, strlen(nodes->node->iden), file);
	fwrite("\n", 1, 1, file);
	nodes = _cluster->nodelisthead;

//	while(node != NULL) {
//	char path[128] = "";
	writechildnodes(file, _cluster->nodelisthead, NULL);
}

void saveclusterdb(clusterlist* _clusterlisthead, char *filename) {
	if(_clusterlisthead != NULL) {
		FILE *cdbfd = fopen(filename, "a+");
		//char buffer[64] = "";
		//char *data;
		//data = fgets(buffer, 64, cdbfd);
		//int sum =  atoi(data);
		clusterlist *cur  = _clusterlisthead;
		while(cur != NULL) {
			if(fwrite(cur->_cluster->clustername, 1, strlen(cur->_cluster->clustername), cdbfd) <= 0) {
				break;
			}
			//++sum;
			char *str = "\nstart\n";
			fwrite(str, 1, strlen(str), cdbfd);
			//char path[1024] = "";
			writenodes(cdbfd, _clusterlisthead->_cluster);
			char *str = "end\n";
			fwrite(str, 1, strlen(str), cdbfd);
			cur = cur->next;
		}
		fclose(cdbfd);
	}
}

cluster *getClusterCopy(cluster *target) {
	cluster *copyCluster = initialcluster(target->clustername);
	node_s_inlist *node = target->nodelisthead;
	while(node != NULL) {
		clusteraddnode(copyCluster, node->node->iden);
		char path[128] = "";
		strcpy(path, node->node->iden);
		copyAllChildren(copyCluster, node->childern, path);
		node = node->next;
	}
	return copyCluster;
}
void copyAllChildren(cluster *copyCluster, node_s_inlist *node, char *path) {
	addnodechild(copyCluster, node->node->iden, path);
	if(node->next) {
		copyAllChildren(copyCluster, node->next, path);
	}
	if(node->childern) {
		char childpath[128] = "";
		strcpy(childpath, path);
		strcat(childpath, ".");
		strcat(childpath, node->node->iden);
		copyAllChildren(copyCluster, node->childern, childpath);
	}
}

char **get_all_leaves(cluster *_cluster) {
	char **ret;
	//at most 10 servers
	ret = malloc(10*sizeof(char*));
	node_s_inlist *listnode = _cluster->nodelisthead;
	int len = 0;
	getleaves(_cluster, &ret, &len);
	return ret;
}

void getleaves(node_s_inlist *nodelist, char ***ret, int *len) {
	if(len >= 10) {
		return;
	}
	if(!nodelist->conhash) {
		ret[len++] = malloc(sizeof(char)*sizeof(nodelist->node->iden));
	}
	if(nodelist->next) {
		getleaves(nodelist->next, &ret, len);
	}
	if(nodelist->childern) {
		getleaves(nodelist->childern, &ret, len);
	}
}
