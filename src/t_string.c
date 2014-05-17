/*
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "redis.h"
#include "cluster/clustermodule/cluster.h"
#include "cluster/clustermodule/hashmap.h"
#include "hiredis.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* isnan(), isinf() */

/*-----------------------------------------------------------------------------
 * String Commands
 *----------------------------------------------------------------------------*/

static int checkStringLength(redisClient *c, long long size) {
	if (size > 512 * 1024 * 1024) {
		addReplyError(c, "string exceeds maximum allowed size (512MB)");
		return REDIS_ERR;
	}
	return REDIS_OK;
}

/* The setGenericCommand() function implements the SET operation with different
 * options and variants. This function is called in order to implement the
 * following commands: SET, SETEX, PSETEX, SETNX.
 *
 * 'flags' changes the behavior of the command (NX or XX, see belove).
 *
 * 'expire' represents an expire to set in form of a Redis object as passed
 * by the user. It is interpreted according to the specified 'unit'.
 *
 * 'ok_reply' and 'abort_reply' is what the function will reply to the client
 * if the operation is performed, or when it is not because of NX or
 * XX flags.
 *
 * If ok_reply is NULL "+OK" is used.
 * If abort_reply is NULL, "$-1" is used. */

#define REDIS_SET_NO_FLAGS 0
#define REDIS_SET_NX (1<<0)     /* Set if key not exists. */
#define REDIS_SET_XX (1<<1)     /* Set if key exists. */

void setGenericCommand(redisClient *c, int flags, robj *key, robj *val,
		robj *expire, int unit, robj *ok_reply, robj *abort_reply) {
	long long milliseconds = 0; /* initialized to avoid any harmness warning */

	if (expire) {
		if (getLongLongFromObjectOrReply(c, expire, &milliseconds,
		NULL) != REDIS_OK)
			return;
		if (milliseconds <= 0) {
			addReplyError(c, "invalid expire time in SETEX");
			return;
		}
		if (unit == UNIT_SECONDS)
			milliseconds *= 1000;
	}

	if ((flags & REDIS_SET_NX && lookupKeyWrite(c->db, key) != NULL)
			|| (flags & REDIS_SET_XX && lookupKeyWrite(c->db, key) == NULL)) {
		addReply(c, abort_reply ? abort_reply : shared.nullbulk);
		return;
	}
	setKey(c->db, key, val);
	server.dirty++;
	if (expire)
		setExpire(c->db, key, mstime() + milliseconds);
	notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "set", key, c->db->id);
	if (expire)
		notifyKeyspaceEvent(REDIS_NOTIFY_GENERIC, "expire", key, c->db->id);
	addReply(c, ok_reply ? ok_reply : shared.ok);
}

/* SET key value [NX] [XX] [EX <seconds>] [PX <milliseconds>] */
void setCommand(redisClient *c) {
	int j;
	robj *expire = NULL;
	int unit = UNIT_SECONDS;
	int flags = REDIS_SET_NO_FLAGS;

	for (j = 3; j < c->argc; j++) {
		char *a = c->argv[j]->ptr;
		robj *next = (j == c->argc - 1) ? NULL : c->argv[j + 1];

		if ((a[0] == 'n' || a[0] == 'N') && (a[1] == 'x' || a[1] == 'X')
				&& a[2] == '\0') {
			flags |= REDIS_SET_NX;
		} else if ((a[0] == 'x' || a[0] == 'X') && (a[1] == 'x' || a[1] == 'X')
				&& a[2] == '\0') {
			flags |= REDIS_SET_XX;
		} else if ((a[0] == 'e' || a[0] == 'E') && (a[1] == 'x' || a[1] == 'X')
				&& a[2] == '\0' && next) {
			unit = UNIT_SECONDS;
			expire = next;
			j++;
		} else if ((a[0] == 'p' || a[0] == 'P') && (a[1] == 'x' || a[1] == 'X')
				&& a[2] == '\0' && next) {
			unit = UNIT_MILLISECONDS;
			expire = next;
			j++;
		} else {
			addReply(c, shared.syntaxerr);
			return;
		}
	}

	c->argv[2] = tryObjectEncoding(c->argv[2]);
	setGenericCommand(c, flags, c->argv[1], c->argv[2], expire, unit, NULL,
	NULL);
}

void setnxCommand(redisClient *c) {
	c->argv[2] = tryObjectEncoding(c->argv[2]);
	setGenericCommand(c, REDIS_SET_NX, c->argv[1], c->argv[2], NULL, 0,
			shared.cone, shared.czero);
}

void setexCommand(redisClient *c) {
	c->argv[3] = tryObjectEncoding(c->argv[3]);
	setGenericCommand(c, REDIS_SET_NO_FLAGS, c->argv[1], c->argv[3], c->argv[2],
	UNIT_SECONDS, NULL, NULL);
}

void psetexCommand(redisClient *c) {
	c->argv[3] = tryObjectEncoding(c->argv[3]);
	setGenericCommand(c, REDIS_SET_NO_FLAGS, c->argv[1], c->argv[3], c->argv[2],
	UNIT_MILLISECONDS, NULL, NULL);
}

int getGenericCommand(redisClient *c) {
	robj *o;

	if ((o = lookupKeyReadOrReply(c, c->argv[1], shared.nullbulk)) == NULL)
		return REDIS_OK;

	if (o->type != REDIS_STRING) {
		addReply(c, shared.wrongtypeerr);
		return REDIS_ERR;
	} else {
		addReplyBulk(c, o);
		return REDIS_OK;
	}
}

void getCommand(redisClient *c) {
	getGenericCommand(c);
}

void getsetCommand(redisClient *c) {
	if (getGenericCommand(c) == REDIS_ERR)
		return;
	c->argv[2] = tryObjectEncoding(c->argv[2]);
	setKey(c->db, c->argv[1], c->argv[2]);
	notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "set", c->argv[1], c->db->id);
	server.dirty++;
}

void setrangeCommand(redisClient *c) {
	robj *o;
	long offset;
	sds value = c->argv[3]->ptr;

	if (getLongFromObjectOrReply(c, c->argv[2], &offset, NULL) != REDIS_OK)
		return;

	if (offset < 0) {
		addReplyError(c, "offset is out of range");
		return;
	}

	o = lookupKeyWrite(c->db, c->argv[1]);
	if (o == NULL) {
		/* Return 0 when setting nothing on a non-existing string */
		if (sdslen(value) == 0) {
			addReply(c, shared.czero);
			return;
		}

		/* Return when the resulting string exceeds allowed size */
		if (checkStringLength(c, offset + sdslen(value)) != REDIS_OK)
			return;

		o = createObject(REDIS_STRING, sdsempty());
		dbAdd(c->db, c->argv[1], o);
	} else {
		size_t olen;

		/* Key exists, check type */
		if (checkType(c, o, REDIS_STRING))
			return;

		/* Return existing string length when setting nothing */
		olen = stringObjectLen(o);
		if (sdslen(value) == 0) {
			addReplyLongLong(c, olen);
			return;
		}

		/* Return when the resulting string exceeds allowed size */
		if (checkStringLength(c, offset + sdslen(value)) != REDIS_OK)
			return;

		/* Create a copy when the object is shared or encoded. */
		if (o->refcount != 1 || o->encoding != REDIS_ENCODING_RAW) {
			robj *decoded = getDecodedObject(o);
			o = createStringObject(decoded->ptr, sdslen(decoded->ptr));
			decrRefCount(decoded);
			dbOverwrite(c->db, c->argv[1], o);
		}
	}

	if (sdslen(value) > 0) {
		o->ptr = sdsgrowzero(o->ptr, offset + sdslen(value));
		memcpy((char*) o->ptr + offset, value, sdslen(value));
		signalModifiedKey(c->db, c->argv[1]);
		notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "setrange", c->argv[1],
				c->db->id);
		server.dirty++;
	}
	addReplyLongLong(c, sdslen(o->ptr));
}

void getrangeCommand(redisClient *c) {
	robj *o;
	long start, end;
	char *str, llbuf[32];
	size_t strlen;

	if (getLongFromObjectOrReply(c, c->argv[2], &start, NULL) != REDIS_OK)
		return;
	if (getLongFromObjectOrReply(c, c->argv[3], &end, NULL) != REDIS_OK)
		return;
	if ((o = lookupKeyReadOrReply(c, c->argv[1], shared.emptybulk)) == NULL
			|| checkType(c, o, REDIS_STRING))
		return;

	if (o->encoding == REDIS_ENCODING_INT) {
		str = llbuf;
		strlen = ll2string(llbuf, sizeof(llbuf), (long) o->ptr);
	} else {
		str = o->ptr;
		strlen = sdslen(str);
	}

	/* Convert negative indexes */
	if (start < 0)
		start = strlen + start;
	if (end < 0)
		end = strlen + end;
	if (start < 0)
		start = 0;
	if (end < 0)
		end = 0;
	if ((unsigned) end >= strlen)
		end = strlen - 1;

	/* Precondition: end >= 0 && end < strlen, so the only condition where
	 * nothing can be returned is: start > end. */
	if (start > end) {
		addReply(c, shared.emptybulk);
	} else {
		addReplyBulkCBuffer(c, (char*) str + start, end - start + 1);
	}
}

void mgetCommand(redisClient *c) {
	int j;

	addReplyMultiBulkLen(c, c->argc - 1);
	for (j = 1; j < c->argc; j++) {
		robj *o = lookupKeyRead(c->db, c->argv[j]);
		if (o == NULL) {
			addReply(c, shared.nullbulk);
		} else {
			if (o->type != REDIS_STRING) {
				addReply(c, shared.nullbulk);
			} else {
				addReplyBulk(c, o);
			}
		}
	}
}

void msetGenericCommand(redisClient *c, int nx) {
	int j, busykeys = 0;

	if ((c->argc % 2) == 0) {
		addReplyError(c, "wrong number of arguments for MSET");
		return;
	}
	/* Handle the NX flag. The MSETNX semantic is to return zero and don't
	 * set nothing at all if at least one already key exists. */
	if (nx) {
		for (j = 1; j < c->argc; j += 2) {
			if (lookupKeyWrite(c->db, c->argv[j]) != NULL) {
				busykeys++;
			}
		}
		if (busykeys) {
			addReply(c, shared.czero);
			return;
		}
	}

	for (j = 1; j < c->argc; j += 2) {
		c->argv[j + 1] = tryObjectEncoding(c->argv[j + 1]);
		setKey(c->db, c->argv[j], c->argv[j + 1]);
		notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "set", c->argv[j], c->db->id);
	}
	server.dirty += (c->argc - 1) / 2;
	addReply(c, nx ? shared.cone : shared.ok);
}

void msetCommand(redisClient *c) {
	msetGenericCommand(c, 0);
}

void msetnxCommand(redisClient *c) {
	msetGenericCommand(c, 1);
}

void incrDecrCommand(redisClient *c, long long incr) {
	long long value, oldvalue;
	robj *o, *new;

	o = lookupKeyWrite(c->db, c->argv[1]);
	if (o != NULL && checkType(c, o, REDIS_STRING))
		return;
	if (getLongLongFromObjectOrReply(c, o, &value, NULL) != REDIS_OK)
		return;

	oldvalue = value;
	if ((incr < 0 && oldvalue < 0 && incr < (LLONG_MIN - oldvalue))
			|| (incr > 0 && oldvalue > 0 && incr > (LLONG_MAX - oldvalue))) {
		addReplyError(c, "increment or decrement would overflow");
		return;
	}
	value += incr;
	new = createStringObjectFromLongLong(value);
	if (o)
		dbOverwrite(c->db, c->argv[1], new);
	else
		dbAdd(c->db, c->argv[1], new);
	signalModifiedKey(c->db, c->argv[1]);
	notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "incrby", c->argv[1], c->db->id);
	server.dirty++;
	addReply(c, shared.colon);
	addReply(c, new);
	addReply(c, shared.crlf);
}

void incrCommand(redisClient *c) {
	incrDecrCommand(c, 1);
}

void decrCommand(redisClient *c) {
	incrDecrCommand(c, -1);
}

void incrbyCommand(redisClient *c) {
	long long incr;

	if (getLongLongFromObjectOrReply(c, c->argv[2], &incr, NULL) != REDIS_OK)
		return;
	incrDecrCommand(c, incr);
}

void decrbyCommand(redisClient *c) {
	long long incr;

	if (getLongLongFromObjectOrReply(c, c->argv[2], &incr, NULL) != REDIS_OK)
		return;
	incrDecrCommand(c, -incr);
}

void incrbyfloatCommand(redisClient *c) {
	long double incr, value;
	robj *o, *new, *aux;

	o = lookupKeyWrite(c->db, c->argv[1]);
	if (o != NULL && checkType(c, o, REDIS_STRING))
		return;
	if (getLongDoubleFromObjectOrReply(c, o, &value, NULL) != REDIS_OK
			|| getLongDoubleFromObjectOrReply(c, c->argv[2], &incr, NULL)
					!= REDIS_OK)
		return;

	value += incr;
	if (isnan(value) || isinf(value)) {
		addReplyError(c, "increment would produce NaN or Infinity");
		return;
	}
	new = createStringObjectFromLongDouble(value);
	if (o)
		dbOverwrite(c->db, c->argv[1], new);
	else
		dbAdd(c->db, c->argv[1], new);
	signalModifiedKey(c->db, c->argv[1]);
	notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "incrbyfloat", c->argv[1],
			c->db->id);
	server.dirty++;
	addReplyBulk(c, new);

	/* Always replicate INCRBYFLOAT as a SET command with the final value
	 * in order to make sure that differences in float precision or formatting
	 * will not create differences in replicas or after an AOF restart. */
	aux = createStringObject("SET", 3);
	rewriteClientCommandArgument(c, 0, aux);
	decrRefCount(aux);
	rewriteClientCommandArgument(c, 2, new);
}

void appendCommand(redisClient *c) {
	size_t totlen;
	robj *o, *append;

	o = lookupKeyWrite(c->db, c->argv[1]);
	if (o == NULL) {
		/* Create the key */
		c->argv[2] = tryObjectEncoding(c->argv[2]);
		dbAdd(c->db, c->argv[1], c->argv[2]);
		incrRefCount(c->argv[2]);
		totlen = stringObjectLen(c->argv[2]);
	} else {
		/* Key exists, check type */
		if (checkType(c, o, REDIS_STRING))
			return;

		/* "append" is an argument, so always an sds */
		append = c->argv[2];
		totlen = stringObjectLen(o) + sdslen(append->ptr);
		if (checkStringLength(c, totlen) != REDIS_OK)
			return;

		/* If the object is shared or encoded, we have to make a copy */
		if (o->refcount != 1 || o->encoding != REDIS_ENCODING_RAW) {
			robj *decoded = getDecodedObject(o);
			o = createStringObject(decoded->ptr, sdslen(decoded->ptr));
			decrRefCount(decoded);
			dbOverwrite(c->db, c->argv[1], o);
		}

		/* Append the value */
		o->ptr = sdscatlen(o->ptr, append->ptr, sdslen(append->ptr));
		totlen = sdslen(o->ptr);
	}
	signalModifiedKey(c->db, c->argv[1]);
	notifyKeyspaceEvent(REDIS_NOTIFY_STRING, "append", c->argv[1], c->db->id);
	server.dirty++;
	addReplyLongLong(c, totlen);
}

void strlenCommand(redisClient *c) {
	robj *o;
	if ((o = lookupKeyReadOrReply(c, c->argv[1], shared.czero)) == NULL
			|| checkType(c, o, REDIS_STRING))
		return;
	addReplyLongLong(c, stringObjectLen(o));
}

extern clusterlist *_clusterlisthead;

/*
 * Store the socket file discriptor.
 * key: 'ip:port'
 * value: sfd
 */
extern hmap_t socketmap;

int argcintlen(int i) {
	int len = 0;
	if (i < 0) {
		len++;
		i = -i;
	}
	do {
		len++;
		i /= 10;
	} while (i);
	return len;
}
size_t argvbulklen(size_t len) {
	return 1 + argcintlen(len) + 2 + len + 2;
}
int appendleftCommandArgv(char **target, int argc, const char **argv,
		const size_t *argvlen) {
	//char *cmd = NULL; /* final command */
	int pos; /* position in final command */
	size_t len;
	int totlen, j;

	int first = argcintlen(argc);
	/* Calculate number of bytes needed for the command */
	totlen = 1 + first + 2;
	for (j = 0; j < argc; j++) {
		len = argvlen ? argvlen[j] : strlen(argv[j]);
		totlen += argvbulklen(len);
	}

	/* Build the command at protocol level */
	char* cmd = "";
	cmd = malloc(totlen + 1);
//    if (cmd == NULL)
//        return -1;

	pos = sprintf(cmd, "*%d\r\n", argc);
	for (j = 0; j < argc; j++) {
		len = argvlen ? argvlen[j] : strlen(argv[j]);
		pos += sprintf(cmd + pos, "$%d\r\n", len);
		memcpy(cmd + pos, argv[j], len);
		pos += len;
		cmd[pos++] = '\r';
		cmd[pos++] = '\n';
	}
	//assert(pos == totlen);
	cmd[pos] = '\0';

	*target = cmd;
	return totlen;
}
/*
 * Process cluster-related command,
 * first will get the cluster and send redis command to target server
 */
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
	} else if(c->argc >= 4 && strcmp((char*) c->argv[2]->ptr, "add") == 0) {

	}else {
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
// /*
//  * Create a new socket and connect to the socket.
//  * Will add to the hashmap if success.
//  */
// int createConnectSocket(char *server) {
// 	char buf[32];
// 	strcpy(buf, server);
// 	int sfd, rv;
// 	char *addr = strtok(buf, ":");
// 	char *_port = strtok(NULL, ":");
// 	struct addrinfo hints, *servinfo, *p;
// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;

//    if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
//             hints.ai_family = AF_INET6;
//             if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
//                printf("error address\n");
//                return -1;
//            }
//        }
//    p = servinfo;
//    if ((sfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
//    	printf("error create socket\n");
//        return -1;
//    }
//    if (connect(sfd,p->ai_addr,p->ai_addrlen) == -1){
//        	printf("connect error\n");
//        	return -1;
//    }
//    return sfd;
// }

/*
 * Convert the arguments in the client buffer to the right protocal of redis.
 */
//getClusterProtocalCommand(redisClient *c) {
//	char cmdBuffer[1024] = "";
//	int length = 3;
//	cmdBuffer[0] = '*';
//	cmdBuffer[1] = c->argc-2+'0';
//	cmdBuffer[2] = '$';
//	int count = 2;
////	int sum = c->argc;
//	while(count < c->argc) {
//		strcpy(cmdBuffer+length, "\r\n");
//		length += 2;
//		int len = strlen((char *)c->argv[count]->ptr);
//		cmdBuffer[length++] = len;
//		strcpy(cmdBuffer+length, (char *)c->argv[count]->ptr);
//		length += len;
//	}
//}
