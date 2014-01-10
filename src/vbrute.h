#define ITEMS_MAX 512
#define ITEM_LENGTH_MAX 512

typedef struct Items Items;
typedef struct Config Config;
   
struct Items {
   char domain[ITEMS_MAX][ITEM_LENGTH_MAX];
   char ip[ITEMS_MAX][ITEM_LENGTH_MAX];
   int domainPtr;
   int ipPtr;

   Config *config;

   struct evhttp_connection *conn;
   struct evhttp_request *req;
   int conn_finished;
};

struct Config {
   /* Timeout struct */
   struct timeval tv;
};
