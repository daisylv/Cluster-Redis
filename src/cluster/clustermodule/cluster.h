/*
 * cluster.h
 *
 *  Created on: Apr 18, 2014
 *      Author: daisy
 */

#ifndef CLUSTER_H_
#define CLUSTER_H_

#include "../libconhash/configure.h"

typedef struct node_s_inlist {
	struct node_s *node;
	struct node_s_inlist *next;
	struct conhash_s *conhash;
	struct node_s_inlist *childern;
} node_s_inlist;

typedef struct cluster {
	char* clustername;
	node_s_inlist* nodelisthead;//head
	struct conhash_s *conhash;
} cluster;

typedef struct clusterlist {
	struct cluster *_cluster;
	struct clusterlist *next;
} clusterlist;

cluster* initialcluster(char *name);
void clusteraddnode(cluster *_cluster, char *nodenamelist);
void removeclusternode(cluster *_cluster, char * nodenamelist);
void addnodechild(cluster *_cluster, char *target, char *childnodenamelist);
void delnodechild(cluster *_cluster, char *target, char *childnodenamelist);
const char * getserver(cluster *_cluster, char *key);

void LoadClusterData(clusterlist* _clusterlisthead, char *filename);

void SaveClusterDB(clusterlist* _clusterlisthead, char *filename);

#endif /* CLUSTER_H_ */
