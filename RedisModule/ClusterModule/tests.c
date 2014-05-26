/*
 * tests.c
 *
 *  Created on: Apr 22, 2014
 *      Author: daisy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cluster.h"

//clusterlist *_clusterlisthead, *_clusterlisttail;

//void LoadClusterData(char *filename) {
//	_clusterlisthead = NULL;
//	_clusterlisttail = NULL;
//	FILE *fd;
//	fd = fopen(filename, "r");
//	if (fd == NULL){
//		printf("error when loading cluster data...");
//		return;
//	}
//	char buffer[256] = "";
//	fgets(buffer, 256, fd);
//	int num = atoi(buffer);
//	if (num == 0)
//		return;
//	int i;
//	for(i = 0; i < num; ++i) {
//		fgets(buffer, 256, fd);
//		if (strcmp(buffer, "start\n") != 0) {
//			continue;
//		}
//		if (strcmp(buffer, "end\n") == 0)
//			continue;
//		fgets(buffer, 256, fd);
//		int len = strlen(buffer);
//		buffer[len-1] = '\0';
//		char clustername[64]="";
//		strcpy(clustername, buffer);
//		cluster *_cluster = initialcluster(clustername);
//
//		fgets(buffer, 256, fd);
//		len = strlen(buffer);
//		buffer[len-1] = '\0';
//		char groups[128];
//		strcpy(groups, buffer);
//		clusteraddnode(_cluster, groups);
//		while (fgets(buffer, 256, fd) != NULL) {
//			len = strlen(buffer);
//			buffer[len-1] = '\0';
//			char nodeinfo[128] = "";
//			strcpy(nodeinfo, buffer);
//			char *parent = strtok(nodeinfo, " ");
//			char *child = strtok(NULL, " ");
//			while (child != NULL) {
//				addnodechild(_cluster, child, parent);
//				child = strtok(NULL, " ");
//			}
//		}
//		if(_clusterlisthead == NULL) {
//			_clusterlisthead = (clusterlist*)malloc(sizeof(clusterlist));
//			_clusterlisthead->_cluster = _cluster;
//			_clusterlisthead->next = NULL;
//			_clusterlisttail = _clusterlisthead;
//		}
//		else {
//			clusterlist *newcluster = (clusterlist*)malloc(sizeof(clusterlist));
//			newcluster->_cluster = _cluster;
//			newcluster->next = NULL;
//			_clusterlisttail->next = newcluster;
//			_clusterlisttail = _clusterlisttail->next;
//		}
//	}
//}


int main() {
//	char name[128];
//	gets(name);
//	printf("start loading MyCluster...\n");
//	LoadClusterData("/home/daisy/Desktop/MyCluster.cdb");
//	printf("MyCluster loading finished\n");
//	printf("type 'start' to begin the test\n");
//	gets(name);
//	if(_clusterlisthead != NULL) {
//		char *name = _clusterlisthead->_cluster->clustername;
//		printf("success in loading the cluster\n");
//		printf("cluster name: %s\n", name);
//	}
	//printf("please input the name of cluster");
	//char clustername[20] = "";
	//scanf("%s", clustername);//"MyCluster";
//	printf("enter the name for the cluster\n");
//	char clustername[100];
//	gets(clustername);
//	cluster* _cluster = initialcluster("MyCluster");//(clustername);
//	printf("initial cluster successfully...\n");
//	printf("cluster name: MyCluster\n");
//	printf("please continue to add node to MyCluster, and free to enjoy!\n");
//	gets(clustername);
//	//char clusternodes[128] = "";
//	//while(scanf("%s", clusternodes)) {
//		clusteraddnode(_cluster, "Group1 Group2 Group3");//clusternodes);
//		printf("add node Group1 successfully\n");
//		printf("add node Group2 successfully\n");
//		printf("add node Group3 successfully\n");
//	//}
//	//scanf("%s", clusternodes);
//		gets(clustername);
//	addnodechild(_cluster, "127.0.0.1:6379", "Group1");
//	printf("add child 127.0.0.1:6379 successfully\n");
//	gets(clustername);
//	addnodechild(_cluster, "127.0.0.1:6380", "Group2");
//	printf("add child 127.0.0.1:6380 successfully\n");
//	gets(clustername);
//	addnodechild(_cluster, "127.0.0.1:6381", "Group3");
//	printf("add child 127.0.0.1:6381 successfully\n");
//	gets(clustername);
//	//printf("will persist the cluster information");
//	int i  = 0;
//	while(i++ < 3) {
//		printf(".");
//	}
//	printf("\n");
//	printf("MyCluster.cdb saved!\n");
//	gets(clustername);
//	delnodechild(_cluster, "127.0.0.1:6381", "Group3.");
//	printf("remove 127.0.0.1:6381 successfully, please save the cluster info in time\n");
//	gets(clustername);
//	while(i++ < 6) {
//			printf(".");
//		}
//	printf("MyCluster.cdb saved!\n");
//	gets(clustername);
////
//	char key[128]="";
//	int i;
//	printf("start getting the leaf node of keys in cluster...\n");
//	for (i = 0; i < 20; i++) {
//		sprintf(key, "key%d", i);
//		printf("%s is in: %s\n", key, getserver(_clusterlisthead->_cluster, key));
//	}
//	printf("end getting the leaf node of 10 keys in cluster\n");
//	gets(name);
//		delnodechild(_clusterlisthead->_cluster, "127.0.0.1:6381", "Group3.");
//		printf("remove 127.0.0.1:6381 successfully, please save the cluster info in time\n");
//		gets(name);
//		printf("start getting the leaf node of keys in cluster...\n");
//			for (i = 0; i < 20; i++) {
//				sprintf(key, "key%d", i);
//				printf("%s is in: %s\n", key, getserver(_clusterlisthead->_cluster, key));
//			}
//			printf("end getting the leaf node of 10 keys in cluster\n");
//			gets(name);


	cluster *mycluster = initialcluster("test");
	clusteraddnode(mycluster, "group1");
	clusteraddnode(mycluster, "group2 group3");
	addnodechild(mycluster, "127.0.0.1:6379", "group1");
	addnodechild(mycluster, "127.0.0.1:6380", "group1");
	addnodechild(mycluster, "127.0.0.1:6381", "group2");
	addnodechild(mycluster, "g3child1", "group3");
	addnodechild(mycluster, "g3child2", "group3");
	addnodechild(mycluster, "127.0.0.1:6382", "group3.g3child1");
	addnodechild(mycluster, "127.0.0.1:6383", "group3.g3child2");

	char buffer[128] = "";
	sprintf(buffer, "/home/daisy/Desktop/%s.cdb", mycluster->clustername);

	clusterlist * clusterlist = malloc(sizeof(clusterlist));
	clusterlist->_cluster = mycluster;

	saveclusterdb(clusterlist, buffer);
	return 0;
}

