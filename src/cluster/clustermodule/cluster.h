/*
 * cluster.h
 *
 *  Created on: Apr 18, 2014
 *      Author: daisy
 */

#ifndef CLUSTER_H_
#define CLUSTER_H_

#include "../libconhash/configure.h"

/* Nodes a list, only to keep the information of the cluster nodes.
 * The actual hashing is by rb-tree, see the data type in conhash.h
 */
typedef struct node_s_inlist {
	struct node_s *node;
	struct node_s_inlist *next;
	struct conhash_s *conhash;
	struct node_s_inlist *childern;
} node_s_inlist;

/* Cluster, contains name, child 
 nodes for hashing, and hash structure to handle*/
typedef struct cluster {
	char clustername[64];
	node_s_inlist* nodelisthead;
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
void delcluster(char *name);

clusterlist* loadClusterData(char *filename);

void saveclusterdb(clusterlist* _clusterlisthead, char *filename);

#endif /* CLUSTER_H_ */
