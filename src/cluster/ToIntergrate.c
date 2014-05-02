void clusterCommand(redisClient *c) {
	// is this argv[1]?
	char *clusname = c->argv[1];
	if (c->curcluster == NULL || strcmp(c->curcluster->clustername, clusname) != 0) {
		cur = _clusterlist;
		while (cur != NULL) {
			if (strcmp(_clusterlist->_cluster->clustername, clusname) == 0) {
				break;
			}
		}
		printf("error: cluster is not exist...\n");
		return;
	}
	redirectServer(c->hashmap, c->argv[2]);
}

void redirectServer(hmap_t hashmap, char *key) {
	if (hashmap == NULL) {
		hashmap = hashmap_create();
	}
	hash_pair_t *elem;
	if(hashmap_get(hashmap, key, elem) != HMAP_S_OK) {
		//create socket and put into hashmap
		char _port[6];
		struct addrinfo hints, *serverinfo, *p;

	}

}