void clusterCommand(redisClient *c) {
	printf("will redirect...");
	printf("is redirect...");
	//char *name = _clusterlisthead->_cluster->clustername;
	clusterlist* targetCluster = _clusterlisthead;
	if (strcmp((char*) c->argv[1]->ptr, "new") == 0) {
		//will add new cluster
		clusterlist *newCluster_in_list = (clusterlist*) malloc(
				sizeof(clusterlist));
		newCluster_in_list->_cluster = initialcluster((char*) c->argv[2]->ptr);
		if (targetCluster == NULL) {
			_clusterlisthead = newCluster_in_list;
			return;
		} else {
			while (targetCluster->next != NULL) {
				targetCluster = targetCluster->next;
			}
			targetCluster->next = newCluster_in_list;
		}
		return;
	}
	while (targetCluster) {
		if (strcmp((char*) c->argv[1]->ptr,
				targetCluster->_cluster->clustername) == 0) {
			break;
		}
		targetCluster = targetCluster->next;
	}
	if (!targetCluster) {
		char *reply = "cluster name dose not exist...";
//		strcpy(c->buf, reply);
//		c->bufpos = strlen(reply);
		robj * obj = (robj *) calloc(1, sizeof(robj));
		//char *tmp = (char*)malloc(50*sizeof(char));
		obj->ptr = sdsnew(reply);
		//strcpy(tmp, reply);
//		obj->ptr = tmp;
		obj->encoding = 0;
//		aeCreateFileEvent(server.el, c->fd, AE_WRITABLE,
//		        sendReplyToClient, c);
		addReply(c, obj);
		//write(c->fd, c->buf, c->bufpos);
		//resetClient(c);
		return;
	} else if (c->argc >= 4 && strcmp((char*) c->argv[2]->ptr, "add") == 0) {
		int index = 3;
		char nodebuf[256] = "";
		char* pos = nodebuf;
		while (index < c->argc) {
			strcpy(pos, (char*) c->argv[index]->ptr);
			*(++pos) = " ";
			pos += strlen((char*) c->argv[index]->ptr);
		}
		clusteraddnode(targetCluster->_cluster, nodebuf);
		robj * obj = (robj *) calloc(1, sizeof(robj));
		obj->ptr = sdsnew("Add nodes successfully...");
		obj->encoding = 0;
		addReply(c, obj);
		return;
	} else if (c->argc == 5 && strcmp((char*) c->argv[3]->ptr, "add") == 0) {
		addnodechild(targetCluster->_cluster, (char*) c->argv[4]->ptr,
				(char*) c->argv[2]->ptr);
		robj * obj = (robj *) calloc(1, sizeof(robj));
		obj->ptr = sdsnew("Add nodes successfully...");
		obj->encoding = 0;
		addReply(c, obj);
		return;
	} else if (c->argc >= 3 && strcmp((char*) c->argv[2]->ptr, "editdone") == 0) {
		char buf[128] = "";
		char *str = "begin to migrate data, please wait...";
		strcpy(buf, str);
		write(c->fd, buf, strlen(str));

		robj obj = (robj *)malloc(sizeof(robj));
		obj->encoding = 0;

		if ((childpid = fork()) == 0) {
        	int retval;

        	/* Child */
        	closeListeningSockets(0);

        	retval = dataMigration(targetCluster->_cluster, 
				(char*) c->argv[3]->ptr));

        	if (retval == 0) {
            	obj->ptr = sdsnew("data migrate finished!");
        	}
        	exitFromChild((retval == 0) ? 0 : 1);
    	} else {
        /* Parent */
        server.stat_fork_time = ustime()-start;
        if (childpid == -1) {
            obj->ptr = sdsnew("data migrate error!");
        }
        
		addReply(c, obj); 
		return;
	} else if (c->argc >= 3 && strcmp((char*) c->argv[2]->ptr, "migrate") == 0) {

	} else {
		char server[32] = "";
		strcpy(server,
				getserver(targetCluster->_cluster, (char*) c->argv[3]->ptr));
		//TODO should see if it is connected...
		char *ipaddress = strtok(server, ":");
		int port = atoi(strtok(NULL, ":"));
		if (port == 6379) {
			robj **r = c->argv;
			c->argv = &(c->argv[2]);
			c->argc = c->argc - 2;

			processCommand(c);
			//free(r[0]);
			//free(r[1]);
			return;
		}
		redisContext *context = NULL;
		int ret = hashmap_get(socketmap, server, context);
		if (ret < 0 || !context) {
			struct timeval timeout = { 1, 500000 }; // 1.5 seconds
			context = redisConnectWithTimeout(ipaddress, port, timeout);
//			sfd = getConnectSocket(server);
			hashmap_put(socketmap, server, context);
		}
		redisReply *reply;
		//reply = redisCommand(context, "%s %s %s");
		size_t *argvlen = malloc((c->argc - 2) * sizeof(size_t));

//		char* a = (char*)(c->argv[2]->ptr);
//		printf("%s", a);
//		robj **co = &(c->argv[2]);
//		char *b = (char*)(co[0]->ptr);
//		printf("%s", b);
		char **argv = malloc((c->argc - 2) * sizeof(char*));
		int i = 2;
		while (i < c->argc) {
			char *a = c->argv[i]->ptr;
			argv[i - 2] = a;
			++i;
		}
		int j = 0;
		while (j < c->argc - 2) {
			int len = strlen(argv[j]);
			argvlen[j] = len;
			++j;
		}
		appendleftCommandArgv(&(context->obuf), c->argc - 2, argv, argvlen);

//		if (redisGetReply(context,&reply) != REDIS_OK) {
//			printf("%s", reply->str);
//		}
		char buffer[1024] = "";
		char *command = context->obuf;
		char cmd[100] = "";
		strcpy(cmd, command);
		printf("%s", command);
		printf("%s", cmd);
		send(context->fd, cmd, 1024, 0);
		int length = 0;
		length = read(context->fd, buffer, 1024);
		while (length) {
			if (length < 0) {
				printf("error receiving data\n");
				break;
			}
			int write_len = write(1, buffer, length);
			if (write_len < length) {
				printf("write Failed\n");
				break;
			}
			length = read(context->fd, buffer, 1024);
		}
		return;
	}

}

/*
 * To migrate data, cluster should be the old cluster configure by user
 * clustermigrate is the signal
 *
 * @target keys to hash
 * @socketmap map store all the sockets in the cluster
 */
int dataMigration(cluster *oldcluster, char *target, hmap_t socketmap) 
{
	int count = 0;
	char **servers = get_all_leaves(_cluster, &count);
	int flags = 0;
	int i = 0;
	for(; i < count; ++i) {
		char *server = servers[i];
		int ret = hashmap_get(socketmap, server, context);
		if (ret < 0 || !context) {
			struct timeval timeout = { 1, 500000 }; // 1.5 seconds
			context = redisConnectWithTimeout(ipaddress, port, timeout);
//			sfd = getConnectSocket(server);
			hashmap_put(socketmap, server, context);
		}
		//cluster name migrate
		size_t *argvlen = malloc(sizeof(size_t));
		char *cmd = malloc(8*sizeof(char));
		cmd[7] = '\0';
		argvlen[0] = strlen(cmd);
		char **argv = malloc(1*sizeof(char*));
		argv[0] = cmd;

		appendleftCommandArgv(&(context->obuf), 1, argv, argvlen);
		write(context->fd, context->obuf, 128);

		char buffer[10] = "";
		read(context->fd, buffer, 10);
		if (strcmp(buffer, "done") == 0) {
			flags++;
		}
		if (flags == count) {
			break;
		}
	}
}

/*
 * Implement of data migrate, need to configure the data type and will get protocol-right
 * command to target server instance.
 * 
 * @_newCluster cluster to configure right instance
 * @redisClient current redisClient, store the info of current connected client
 */
int migrate(cluster *_newcluster, redisClient c) {
    dictIterator *di = NULL;
    dictEntry *de;
    int j;
    long long now = mstime();

    for (j = 0; j < server.dbnum; j++) {
        redisDb *db = server.db+j;
        dict *d = db->dict;
        if (dictSize(d) == 0) continue;
        di = dictGetSafeIterator(d);
        if (!di) {
            fclose(fp);
            return REDIS_ERR;
        }

        /* Iterate this DB writing every entry */
        while((de = dictNext(di)) != NULL) {
            sds keystr = dictGetKey(de);
            char newserver;
            
            initStaticStringObject(key,keystr);
            newserver = getserver(_newcluster,&key);
            if(strcmp(newserver, selfServer) == 0) {
            	continue;
            }
            else {
				robj key= dictGetVal(de);
				redisContext *context = NULL;
				int ret = hashmap_get(socketmap, server, context);

				if (ret < 0 || !context) {
					struct timeval timeout = { 1, 500000 }; // 1.5 seconds
					context = redisConnectWithTimeout(ipaddress, port, timeout);
					hashmap_put(socketmap, server, context);
				}

				size_t *argvlen = malloc(3 * sizeof(size_t));
				char **argv = malloc(3 * sizeof(char*));
				getReSendCommand(&argvlen, &argv, keystr, key);

				appendleftCommandArgv(&(context->obuf), c->argc - 2, argv, argvlen);

				send(context->fd, context->obuf, 1024, 0);
            }
        }
        dictReleaseIterator(di);
    }
}

void getReSendCommand(size_t **argvlen, char ***argv) {
	argv[0] = malloc(3*sizeof(char));
	argvlen[0] = 3;
	strcmp(argv[0], "set");
	argv[1] = malloc(strlen(keystr->buf)*sizeof(char));
	argvlen[1] = strlen(keystr->buf);
	strcmp(argv[1], keystr->buf);
	argv[2] = malloc(strlen((char*)key->ptr)*sizeof(char));
	argvlen[2] = strlen((char*)key->ptr);
	strcmp(argv[2], (char*)key->ptr);
}


集群模块：
测试用例1：
测试集群的新建与初始化数据
测试结果：
结论
测试用例2：
测试集群的节点添加与删除
测试结果：
结论
测试用例3：
测试集群的数据载入
测试结果：
结论
测试用例4：
测试集群的数据持久化到磁盘
测试结果：
结论
测试用例5：
测试集群的数据转发机制
测试结果：
结论

一致性哈希模块
测试用例5：
测试一致性哈希虚拟节点的初始化
测试结果：
结论
测试用例6：
测试一致性哈希虚拟节点的添加
测试结果：
结论
测试用例7：
测试一致性哈希虚拟节点的删除
测试结果：
结论
测试用例8：
测试一致性哈希获取服务器实例的稳定性
测试结果：
结论
测试用例9：
测试一致性哈希获取服务器实例的稳定性
测试结果：
结论
数据迁移模块
测试用例11：
测试数据迁移指令获取后广播集群所有实例
测试结果：
结论
测试用例12：
测试数据迁移结果反馈
测试结果：
结论


A.1 系统读入Cluster结构文件进行初始化
A.1.1系统初始化时加载Cluster文件:
A.1.2持久化Cluster信息到文件:
A.1.3新增集群
A.1.4删除集群
A.1.5新增集群节点
A.1.6删除集群节点
A.1.7向集群存入数据
A.1.8从集群删除数据
A.2 数据迁移测试
A.2.1数据迁移命令发送与状态获取
A.2.2 数据迁移结果
A.2.3 数据迁移后的效果：（某节点已经不存在该数据）