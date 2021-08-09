/*
 * (C) Radim Kolar 1997-2004
 * This is free software, see GNU Public License version 2 for
 * details.
 *
 * Simple forking WWW Server benchmark:
 *
 * Usage:
 *   webbench --help
 *
 * Return codes:
 *    0 - sucess
 *    1 - benchmark failed (server is not on-line)
 *    2 - bad param
 *    3 - internal error, fork failed
 * 
 */ 
#include "socket.c"
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>

/* values */ //默认设置
volatile int timerexpired=0;
int speed=0;
int failed=0;
int bytes=0;
/* globals */
int http10=1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */
/* Allow: GET, HEAD, OPTIONS, TRACE */
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGRAM_VERSION "1.5"
int method=METHOD_GET; //GET方式
int clients=1; //只模拟一个客户端
int force=0;  //等待响应
int force_reload=0;  //失败时重新请求
int proxyport=80;  //访问端口
char *proxyhost=NULL; //代理服务器
int benchtime=30; //模拟请求时间
/* internal */
int mypipe[2]; //管道
char host[MAXHOSTNAMELEN]; //网络地址
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE]; //请求

//参数
static const struct option long_options[]=
{
 {"force",no_argument,&force,1},
 {"reload",no_argument,&force_reload,1},
 {"time",required_argument,NULL,'t'},
 {"help",no_argument,NULL,'?'},
 {"http09",no_argument,NULL,'9'},
 {"http10",no_argument,NULL,'1'},
 {"http11",no_argument,NULL,'2'},
 {"get",no_argument,&method,METHOD_GET},
 {"head",no_argument,&method,METHOD_HEAD},
 {"options",no_argument,&method,METHOD_OPTIONS},
 {"trace",no_argument,&method,METHOD_TRACE},
 {"version",no_argument,NULL,'V'},
 {"proxy",required_argument,NULL,'p'},
 {"clients",required_argument,NULL,'c'},
 {NULL,0,NULL,0}
};

/* prototypes */
/*
 *（在子进程中）反复发送http请求 
 * 设置信号，安装闹钟
 * 循环，直到服务器超时
 * 连接服务器，获取fd，更改失败次数
 * 写入http请求，更改失败次数
 * 对HTTP0.9版本做特殊处理
 * 等待服务器响应，反复从fd读入相应的数据，累加，失败更改失败次数
 *关闭fd，记录成功次数
 */
static void benchcore(const char* host,const int port, const char *request);
/**
 * 创建各子进程并发送http请求，并统计数据
 * 检测目标服务器是否可以连接
 * 创建一对管道，分别用作读取端、写入端
 * 创建clients个子进程，在子进程中调用benchcore()进行测试，并向管道写入数据：成功次数、失败次数、字节数
 * 在父进程中：循环clients次，从管道中读取数据并统计相应数据
 * 打印统计结果
 */
static int bench(void);
/**
 *生成HTTP请求(最后将输入的url转换为相应格式的请求) 
 * 获取请求方法、添加空行
 * 判断URL合理性
 * 获取代理服务器的ip和端口号（若无输入地址）
 * 获取http版本
 */
static void build_request(const char *url);
//计时器函数
static void alarm_handler(int signal)
{
   timerexpired=1;
}	

//用法（可打印）
static void usage(void)
{
   fprintf(stderr,
	"webbench [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
	"  -9|--http09              Use HTTP/0.9 style requests.\n"
	"  -1|--http10              Use HTTP/1.0 protocol.\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  --options                Use OPTIONS request method.\n"
	"  --trace                  Use TRACE request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
};

int main(int argc, char *argv[])
{
 int opt=0;
 int options_index=0;
 char *tmp=NULL;

 if(argc==1)
 {
	  usage();
          return 2;
 } 

//输入多个参数
/*
*若字符后带':'，说明其可指定值
*如果longindex非空，它指向的变量将记录当前找到参数符合longopts里的第几个元素的描述，即是longopts的下标值
*optarg: 当前位置的参数，：后面的参数。
*optind: optind表示的是下一个将被处理到的参数在argv中的下标值
*opterr: 错误信息 
*/
 while((opt=getopt_long(argc,argv,"912Vfrt:p:c:?h",long_options,&options_index))!=EOF )
 {
  switch(opt)
  {
   case  0 : break;
   case 'f': force=1;break;
   case 'r': force_reload=1;break; 
   case '9': http10=0;break;
   case '1': http10=1;break;
   case '2': http10=2;break;
   case 'V': printf(PROGRAM_VERSION"\n");exit(0);
   case 't': benchtime=atoi(optarg);break;	     
   case 'p': 
	     /* proxy server parsing server:port */ //使用代理服务器
        //查找optarg中首次出现的':'及之后的字符
	     tmp=strrchr(optarg,':');
	     proxyhost=optarg;
	     if(tmp==NULL)
	     {
		     break;
	     }
	     if(tmp==optarg)
	     {
		     fprintf(stderr,"Error in option --proxy %s: Missing hostname.\n",optarg);
		     return 2;
	     }
	     if(tmp==optarg+strlen(optarg)-1)
	     {
		     fprintf(stderr,"Error in option --proxy %s Port number is missing.\n",optarg);
		     return 2;
	     }
	     *tmp='\0';
	     proxyport=atoi(tmp+1);break;
   case ':':
   case 'h':
   case '?': usage();return 2;break;
   case 'c': clients=atoi(optarg);break;
  }
 }
 
 //缺少参数，缺少URL地址
 if(optind==argc) {
                      fprintf(stderr,"webbench: Missing URL!\n");
		      usage();
		      return 2;
                    }

 //默认客户端、请求时间值
 if(clients==0) clients=1;
 if(benchtime==0) benchtime=60;
 /* Copyright */
 fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n"
	 "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.\n"
	 );
 //构造HTTP请求到request数组
 build_request(argv[optind]);
 /* print bench info */ //以下到函数结束为输出提示信息
 printf("\nBenchmarking: ");
 switch(method)
 {
	 case METHOD_GET:
	 default:
		 printf("GET");break;
	 case METHOD_OPTIONS:
		 printf("OPTIONS");break;
	 case METHOD_HEAD:
		 printf("HEAD");break;
	 case METHOD_TRACE:
		 printf("TRACE");break;
 }
 printf(" %s",argv[optind]);
 switch(http10)
 {
	 case 0: printf(" (using HTTP/0.9)");break;
	 case 2: printf(" (using HTTP/1.1)");break;
 }
 printf("\n");
 if(clients==1) printf("1 client");
 else
   printf("%d clients",clients);

 printf(", running %d sec", benchtime);
 if(force) printf(", early socket close");
 if(proxyhost!=NULL) printf(", via proxy server %s:%d",proxyhost,proxyport);
 if(force_reload) printf(", forcing reload");
 printf(".\n");
 //开始压力测试，并访问测试结果
 return bench();
}
/*
 *输入：URL字符串 
 */
//构建HTTP请求
void build_request(const char *url)
{
  char tmp[10];
  int i;
  //初始化
  bzero(host,MAXHOSTNAMELEN);
  bzero(request,REQUEST_SIZE);
  
  //判断使用HTTP版本
  if(force_reload && proxyhost!=NULL && http10<1) http10=1;
  if(method==METHOD_HEAD && http10<1) http10=1;
  if(method==METHOD_OPTIONS && http10<2) http10=2;
  if(method==METHOD_TRACE && http10<2) http10=2;

  //四种请求方法，选择一个
  switch(method)
  {
	  default:
	  case METHOD_GET: strcpy(request,"GET");break;
	  case METHOD_HEAD: strcpy(request,"HEAD");break;
	  case METHOD_OPTIONS: strcpy(request,"OPTIONS");break;
	  case METHOD_TRACE: strcpy(request,"TRACE");break;
  }
		  
  strcat(request," ");

  //判断URL格式正确性
  if(NULL==strstr(url,"://"))
  {
	  fprintf(stderr, "\n%s: is not a valid URL.\n",url);
	  exit(2);
  }
  //URL是否过长
  if(strlen(url)>1500)
  {
         fprintf(stderr,"URL is too long.\n");
	 exit(2);
  }
  //若不使用代理服务器，判断输入的URL地址是否时HTTP地址（只能使用http地址）
  if(proxyhost==NULL)
	   if (0!=strncasecmp("http://",url,7)) 
	   { fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
             exit(2);
           }
  /* protocol/host delimiter */ //找到域名开始的位置（在url字符串中的位置）
  i=strstr(url,"://")-url+3;
  /* printf("%d\n",i); */
  //域名必须以'/'结尾
  if(strchr(url+i,'/')==NULL) {
                                fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
                                exit(2);
                              }
  //若不使用代理服务器
  if(proxyhost==NULL)
  {
   /* get port from hostname */ //从url地址中解析端口号
   if(index(url+i,':')!=NULL &&
      index(url+i,':')<index(url+i,'/'))
   {
	   strncpy(host,url+i,strchr(url+i,':')-url-i);
	   bzero(tmp,10);
      //得到':'和'/'之间的字符串
	   strncpy(tmp,index(url+i,':')+1,strchr(url+i,'/')-index(url+i,':')-1);
	   /* printf("tmp=%s\n",tmp); */
	   proxyport=atoi(tmp);
	   if(proxyport==0) proxyport=80;
   } else
   {
     /*
     *strncpy函数用于将指定长度的字符串复制到字符数组,语法形式为：char *strncpy(char *dest, const char *src, int n)，表示把src所指向的字符串中以src地址开始的前n个字节复制到dest所指的数组中，并返回被复制后的dest
     *strcspn()从参数s字符串的开头计算连续的字符，而这些字符都完全不在参数reject 所指的字符串中。简单地说， 若strcspn()返回的数值为n，则代表字符串s 开头连续有n 个字符都不含字符串reject 内的字符
     */
    //将"://"和'/'之间的字符串复制到host中
     strncpy(host,url+i,strcspn(url+i,"/"));
   }
   // printf("Host=%s\n",host);
   //将request和url中'/'之后的字符串连起来
   strcat(request+strlen(request),url+i+strcspn(url+i,"/"));
  } else
  {
   // printf("ProxyHost=%s\nProxyPort=%d\n",proxyhost,proxyport);
   strcat(request,url);
  }
  //根据设置的各类值，连接相应的信息
  if(http10==1)
	  strcat(request," HTTP/1.0");
  else if (http10==2)
	  strcat(request," HTTP/1.1");
  strcat(request,"\r\n");
  if(http10>0)
	  strcat(request,"User-Agent: WebBench "PROGRAM_VERSION"\r\n");
  if(proxyhost==NULL && http10>0)
  {
	  strcat(request,"Host: ");
	  strcat(request,host);
	  strcat(request,"\r\n");
  }
  if(force_reload && proxyhost!=NULL)
  {
	  strcat(request,"Pragma: no-cache\r\n");
  }
  if(http10>1)//"Connection: close"表示短连接，一次连接即时关闭
	  strcat(request,"Connection: close\r\n");
  /* add empty line at end */ //添加空行
  if(http10>0) strcat(request,"\r\n"); 
  // printf("Req=%s\n",request);
}

/* vraci system rc error kod */
static int bench(void)
{
  int i,j,k;	
  pid_t pid=0;
  FILE *f;

  /* check avaibility of target server */ //调用Socket，发起一次TCP连接，返回scoket
  i=Socket(proxyhost==NULL?host:proxyhost,proxyport);
  if(i<0) { //连接失败
	   fprintf(stderr,"\nConnect to server failed. Aborting benchmark.\n");
           return 1;
         }
  close(i);
  /* create pipe */ //创建管道，其中mypipe[0]为读取端，mypipe[1]为写入端
  if(pipe(mypipe))
  {
	  perror("pipe failed.");
	  return 3;
  }

  /* not needed, since we have alarm() in childrens */
  /* wait 4 next system clock tick */
  /*
  cas=time(NULL);
  while(time(NULL)==cas)
        sched_yield();
  */

  /* fork childs */ //创建相应个数的子进程
  for(i=0;i<clients;i++)
  {
	   pid=fork();
	   if(pid <= (pid_t) 0)
	   {
		   /* child process or error*/
	           sleep(1); /* make childs faster */
		   break;
	   }
  }

  if( pid< (pid_t) 0) //创建子进程失败
  {
          fprintf(stderr,"problems forking worker no. %d\n",i);
	  perror("fork failed.");
	  return 3;
  }

  if(pid== (pid_t) 0)  //在子进程中，进行http请求
  {
    /* I am a child */
    if(proxyhost==NULL) //若不使用代理服务器
      benchcore(host,proxyport,request); //反复发送http请求
         else
      benchcore(proxyhost,proxyport,request);

         /* write results to pipe */
	 f=fdopen(mypipe[1],"w"); //子进程将测试结果输出（写）到管道mypipe[1]中
	 if(f==NULL)
	 {
		 perror("open pipe for writing failed.");
		 return 3;
	 }
	 /* fprintf(stderr,"Child - %d %d\n",speed,failed); */
    //向管道写入相应的测试结果：成功数、失败次数、字节数
	 fprintf(f,"%d %d %d\n",speed,failed,bytes);
	 fclose(f);
	 return 0;
  } else
  {
     //在父进程中，从管道读取所有子进程的输出，并作汇总
	  f=fdopen(mypipe[0],"r");
	  if(f==NULL) 
	  {
		  perror("open pipe for reading failed.");
		  return 3;
	  }
     //设置数据直接冲流中读入或写入，没有缓冲
	  setvbuf(f,NULL,_IONBF,0);
	  speed=0;
          failed=0;
          bytes=0;

     //循环统计clients个进程的数据信息
	  while(1)
	  {
		  pid=fscanf(f,"%d %d %d",&i,&j,&k);
		  if(pid<2)
                  {
                       fprintf(stderr,"Some of our childrens died.\n");
                       break;
                  }
		  speed+=i;
		  failed+=j;
		  bytes+=k;
		  /* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid); */
		  //当所有子进程的数据都读完了，就退出
        if(--clients==0) break;
	  }
	  fclose(f);
  //将结果打印到屏幕上
  printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d susceed, %d failed.\n",
		  (int)((speed+failed)/(benchtime/60.0f)), //每分钟的请求数
		  (int)(bytes/(float)benchtime),  //每秒服务器返回的字节数
		  speed,  //成功请求数
		  failed);  //失败请求数
  }
  return i;
}

void benchcore(const char *host,const int port,const char *req)
{
 int rlen;
 char buf[1500]; //保存服务器的响应报文
 int s,i;
 struct sigaction sa;

 /* setup alarm signal handler */ //注册SIGALRM信号的信号处理函数
 sa.sa_handler=alarm_handler;
 sa.sa_flags=0;
 if(sigaction(SIGALRM,&sa,NULL))
    exit(3);
 alarm(benchtime); //计时，若超时产生SIGALRM信号给本函数

 rlen=strlen(req); //计算请求报文大小
 nexttry:while(1)
 {
    if(timerexpired) //若超时，则返回
    {
       if(failed>0)
       {
          /* fprintf(stderr,"Correcting failed by signal\n"); */
          failed--;
       }
       return;
    }
    s=Socket(host,port);  //调用Socket函数建立TCP连接                          
    if(s<0) { failed++;continue;} //TCP连接建立失败
    if(rlen!=write(s,req,rlen)) {failed++;close(s);continue;} //请求报文前后长度不一致，失败并关闭连接
    if(http10==0) 
	    if(shutdown(s,1)) { failed++;close(s);continue;} //调用shutdown使客户端不能在向服务端发送数据，0成功，-1失败
    if(force==0)  //全局变量force表示是否要等待服务器返回的数据
    {
            /* read all available data from socket */
	    while(1)
	    {
              if(timerexpired) break; //若超时，则返回
	      i=read(s,buf,1500); //读取从服务器返回的数据，保存到buf中
              /* fprintf(stderr,"%d\n",i); */
	      if(i<0) //若读取失败，则关闭连接
              { 
                 failed++;
                 close(s);
                 goto nexttry;
              }
	       else
		       if(i==0) break;
		       else
			       bytes+=i;
	    }
    }
    if(close(s)) {failed++;continue;}
    speed++;
 }
}
