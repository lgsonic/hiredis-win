#include <stdio.h>
#include <tchar.h>

#include <hiredis.h>
#include <winsock2.h>

int _tmain(int argc, _TCHAR* argv[])
{
	redisContext *c;
	redisReply *reply;
	const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
	int port = (argc > 2) ? atoi(argv[2]) : 6379;

	WSADATA	wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(hostname, port, timeout);
	if (c == NULL || c->err) {
		if (c) {
			printf("Connection error: %s\n", c->errstr); 
			redisFree(c);
		}
		else {
			printf("Connection error: can't allocate redis context\n");
		}
		exit(1);
	}

	/* PING server */
	reply = (redisReply*)redisCommand(c, "PING");
	printf("PING: %s\n", reply->str);
	freeReplyObject(reply);

	/* Set a key */
	reply = (redisReply*)redisCommand(c, "SET %s %s", "foo", "hello world");
	printf("SET: %s\n", reply->str);
	freeReplyObject(reply);

	/* Set a key using binary safe API */
	reply = (redisReply*)redisCommand(c, "SET %b %b", "bar", (size_t)3, "hello", (size_t)5);
	printf("SET (binary API): %s\n", reply->str);
	freeReplyObject(reply);

	/* Try a GET and two INCR */
	reply = (redisReply*)redisCommand(c, "GET foo");
	printf("GET foo: %s\n", reply->str);
	freeReplyObject(reply);

	reply = (redisReply*)redisCommand(c, "INCR counter");
	printf("INCR counter: %lld\n", reply->integer);
	freeReplyObject(reply);
	/* again ... */
	reply = (redisReply*)redisCommand(c, "INCR counter");
	printf("INCR counter: %lld\n", reply->integer);
	freeReplyObject(reply);

	/* Create a list of numbers, from 0 to 9 */
	reply = (redisReply*)redisCommand(c, "DEL mylist");
	freeReplyObject(reply);
	for (int j = 0; j < 10; j++)
	{
		reply = (redisReply*)redisCommand(c, "LPUSH mylist element-%d", j);
		freeReplyObject(reply);
	}

	/* Let's check what we have inside the list */
	reply = (redisReply*)redisCommand(c, "LRANGE mylist 0 -1");
	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t j = 0; j < reply->elements; j++)
		{
			printf("%u) %s\n", j, reply->element[j]->str);
		}
	}
	freeReplyObject(reply);

	return 0;
}
