#include <event.h>
#include <evhttp.h>
#include <getopt.h>
#include <netdb.h> /* hostent */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_SIZE_DOMAIN 128
#define DEF_SIZE_FILENAME 128

#include "../src/vbrute.h"

int argDebug, argTimeout = 5;
char argDomainFile[DEF_SIZE_FILENAME], argIPFile[DEF_SIZE_FILENAME];

int debugPrintf(char msg[1024], ...)
{
   if (argDebug == 1)
   {
      va_list args;
      va_start(args, msg);

      vprintf(msg, args);

      va_end(args);
   }

   return 0;
}

int printResult(struct evhttp_request *req, Items *items)
{
   /* domain, ip, response code, lengeth */
   printf("%-24.24s %-16.16s %-10u %-10.10s\n",items->domain[items->domainPtr],items->ip[items->ipPtr],req->response_code,evhttp_find_header(req->input_headers, "Content-Length"));
}

void reqhandler(struct evhttp_request *req, void *vItems)
{
   /* Typecast */
   Items *items = (Items *) vItems;

   items->conn_finished = 1;

   if (req == NULL) {
      debugPrintf("timed out!\n");
   } else if (req->response_code == 0) {
      debugPrintf("connection refused!\n");
   } else {
      debugPrintf("success: %u %s\n", req->response_code, req->response_code_line);
      debugPrintf("var == %s\n", evhttp_find_header(req->input_headers, "Content-Length"));
      printResult(req,items);
   }
   event_loopexit(NULL);
}

void process_timeout(int fd, short event, void *vItems)
{
   Items *items = (Items *) vItems;
   printf("%-24.24s %-16.16s *** Timed out ***\n",items->domain[items->domainPtr],items->ip[items->ipPtr]);

   if (items->conn_finished == 0) {
      evhttp_cancel_request(items->req);
   }
}

int doHTTPRequest(Items *items)
{
   struct event ev;
   unsigned int port = 80;

   items->conn_finished = 0;

   debugPrintf("initializing libevent subsystem..\n");
   event_init();

   items->conn = evhttp_connection_new(items->ip[items->ipPtr], port);
   /* If user sets timeout > than this then the user will not know the site timed out ... */
   evhttp_connection_set_timeout(items->conn, 120);

   items->req = evhttp_request_new(reqhandler, items);
   evhttp_add_header(items->req->output_headers, "Host", items->domain[items->domainPtr]);
   evhttp_add_header(items->req->output_headers, "Content-Length", "0");
   evhttp_make_request(items->conn, items->req, EVHTTP_REQ_GET, "/");

   evtimer_set(&ev, process_timeout, (void*) items);
   evtimer_add(&ev, &items->config->tv);

   event_dispatch();

   return 0;
}

/*
   1. Read in IP list and domain file
   2. Visit each domain using IP list
   3. Record response length and code for diff

   Need HTTPS support
*/

char readFile(char *fileName, char itemArray[ITEMS_MAX][ITEM_LENGTH_MAX])
{
   FILE *file;
   int c;

   if (!(file = fopen(fileName, "r")))
   {
      /* TODO Error function */
      printf("Could not open file\n");
      exit(1);
   }

   while(fgets(itemArray[c++],sizeof(itemArray[ITEMS_MAX]),file))
   {
      itemArray[c-1][strnlen(itemArray[c-1],ITEM_LENGTH_MAX)-1] = '\0';
      debugPrintf("readFile itemArray[c] == %s\n", itemArray[c-1]);
   }

   fclose(file);

   /* Array size */
   return c-1;
}

int showHelp()
{
   printf("vbrute --domain-file <domain-file> --ip-file <ip-file>\n\n");
   printf("-d, --domain-file   Specify file containing a list of domains you want to test\n");
   printf("-i, --ip-file       Specify file containing a list of IP addresses you want to test\n");
   printf("-t, --timeout       Specify request timeout. Default is 5 seconds\n");

   exit(1);
}

int assignArgs(int argc, char **argv)
{
   /* Flag set by '--verbose'. */
   static int argVerboseFlag;

   int c;
     
   while (1)
   {
      static struct option long_options[] =
      {
         /* These options set a flag. */
         {"verbose",     no_argument,       &argVerboseFlag, 1},
         {"debug",       no_argument,       &argDebug,       1},
         //{"brief",       no_argument,       &argVerboseFlag,    0},
         /* These options don't set a flag.
            We distinguish them by their indices. */
         {"timeout",      required_argument, 0, 't'},
         {"domain-file",  required_argument, 0, 'd'},
         {"ip-file",      required_argument, 0, 'i'},
         {0, 0, 0, 0}
      };

      /* getopt_long stores the option index here. */
      int option_index = 0;
     
      c = getopt_long(argc, argv, "t:d:i:", long_options, &option_index);

      debugPrintf("assignArgs argc == %d\n", argc);

      /* domain is only required argument (ip can be determined via domain) */
      if (argc == 1)
      {
         /* Only binary called */
         showHelp();
      }
 
      /* Detect the end of the options. */
      if (c == -1)
         break;
     
      switch (c)
      {
         case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
               break;

            debugPrintf("option %s", long_options[option_index].name);
            if (optarg)
               debugPrintf(" with arg %s", optarg);
            debugPrintf("\n");
            break;

         case 't':
            debugPrintf("option -t with value `%s'\n", optarg);
            /* Str to int TODO */
            char *endptr;
            argTimeout = strtol(optarg,&endptr,10);
            break;

         case 'd':
            debugPrintf("option -d with value `%s'\n", optarg);
            strncpy(argDomainFile, optarg, DEF_SIZE_FILENAME);
            break;

         case 'i':
            debugPrintf("option -i with value `%s'\n", optarg);
            strncpy(argIPFile, optarg, DEF_SIZE_FILENAME);
            break;

         case '?':
            /* getopt_long already printed an error message. */
            break;

         default:
            debugPrintf("assignArgs abort to be called\n");
            abort();
      }
   }

   /* Instead of reporting '--verbose'
      and '--brief' as they are encountered,
      we report the final status resulting from them. */
   if (argVerboseFlag)
      puts ("verbose flag is set");
    
   /* Print any remaining command line arguments (not options). */
   if (optind < argc)
   {
      debugPrintf ("non-option ARGV-elements: ");
      while (optind < argc)
         debugPrintf ("%s ", argv[optind++]);
      putchar ('\n');
   }

   /* Required arguments */
   if (argDomainFile[0] == '\0' || argIPFile[0] == '\0')
   {
      /* Only binary called */
      showHelp();
   }
 
   return 0;
}

int main(int argc, char **argv)
{
   Items items;
   Config config;

   argDomainFile[0] = '\0';
   argIPFile[0] = '\0';

   assignArgs(argc,argv);

   items.config = &config;

   items.config->tv.tv_sec = argTimeout;
   items.config->tv.tv_usec = argTimeout * 1000;

   char domainFile[ITEM_LENGTH_MAX], ipFile[ITEM_LENGTH_MAX];
   int i, j, domainArraySize, ipArraySize;

   /* domain is 2d array */
   domainArraySize = readFile(argDomainFile,items.domain);
   /* ip is 2d array */
   ipArraySize = readFile(argIPFile,items.ip);

   printf("vbrute v0.1 by Aidan Marlin\n");

   /* for ips in file */
   for(items.ipPtr=0;items.ipPtr<ipArraySize;items.ipPtr++)
   {
      /* domain, ip, response code, lengeth */
      printf("--------------------------------------------------------------\n");
      printf("Domain                   IP               Code      Length\n");
      printf("--------------------------------------------------------------\n");
      /* for domains in file */
      for(items.domainPtr=0;items.domainPtr<domainArraySize;items.domainPtr++)
      {
         doHTTPRequest(&items);
      }
   }

   if (argDebug == 1)
   {
      printf("Domains\n");
      for(i=0;i<domainArraySize;i++)
      {
         printf("%s\n", items.domain[i]);
      }

      printf("IPs\n");
      for(i=0;i<domainArraySize;i++)
      {
         printf("%s\n", items.ip[i]);
      }
   }

   return 0;
}
