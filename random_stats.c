#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>

#ifdef DEBUG
#define PRINTD(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define PRINTD(fmt, ...)
#endif

#define NSECS 1000000000.0
#define USECS 1000000.0
#define MSECS 1000.0
#define SECS  1.0

void method1(int *stats);
void method2(int *stats);
void method3(int *stats);

int P = 100000; // pool of total points
int X = 400;    // number of stats
int N = 1;      // number of iterations
char *unit_name = "ns";
double units = NSECS;
int pretty_print = 0;
int quiet = 0;

int main(int argc, char *argv[])
{
  static int unit_flag;
  while (1) {
    static struct option long_options[] =
      {
        {"pretty-print", no_argument, &pretty_print, 1},
        {"ns", no_argument, &unit_flag, 1},
        {"us", no_argument, &unit_flag, 2},
        {"ms", no_argument, &unit_flag, 3},
        {"s", no_argument, &unit_flag, 4},
        {0,0,0,0}
      };
    /* Parse cmd arguments if provided */
    int c, option_index = 0;

    c = getopt_long(argc, argv, "x:p:n:q", long_options, &option_index);
    if (c == -1) break;
    
    switch (c) {
    case 0:
      break;
    case 'x':
      if (optopt == 'x')
        fprintf(stderr, "Option x needs a number\n");
      else
        X = atoi(optarg);
      break;
    case 'p':
      if (optopt == 'p')
        fprintf(stderr, "Option p needs a number\n");
      else
        P = atoi(optarg);
      break;
    case 'n':
      if (optopt == 'n')
        fprintf(stderr, "Option n needs a number\n");
      else
        N = atoi(optarg);
      break;
    case 'q':
      quiet = 1;
      break;
    default:
      break;
    }
  }

  switch (unit_flag) {
  case 0:
  case 1:
    units = NSECS;
    unit_name = "ns";
    break;
  case 2:
    units = USECS;
    unit_name = "us";
    break;
  case 3:
    units = MSECS;
    unit_name = "ms";
    break;
  case 4:
    units = SECS;
    unit_name = "s";
    break;
  }
  

  printf("X = %d, P = %d\n", X, P);
  
  time_t t;
  /* Initialize stats to 0 */
  int statblock1[X];
  for (int i = 0; i < X; ++i) statblock1[i] = 0;
  int statblock2[X];
  for (int i = 0; i < X; ++i) statblock2[i] = 0;
  int statblock3[X];
  for (int i = 0; i < X; ++i) statblock3[i] = 0;
  
  srand(time(&t));

  /* Maybe this is dumb, benchmarking is hard okay?
     Doing it like https://stackoverflow.com/a/41959179 */
  
  struct timespec t1b, t1e, t2b, t2e, t3b, t3e;
  double lt1, tt1, lt2, tt2, lt3, tt3;
  

  for (int j = 0; j < N; j++) {
    if (!quiet) printf("Running method 1...\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1b);
    method1(statblock1);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1e);
    lt1 = (t1e.tv_nsec - t1b.tv_nsec) / NSECS + (t1e.tv_sec - t1b.tv_sec);
    tt1 += lt1;
    if (!quiet) printf("Time = %fs\n", lt1);

    if (pretty_print) {
      printf("STATBLOCK 1\n");
      for (int i = 0; i < X; ++i) {
        printf("Stat %d -> %d\n", i, statblock1[i]);
      }
      printf("---------------------\n");
    }
  
    if (!quiet) printf("Running method 2...\n");
  
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2b);
    method2(statblock2);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2e);
    lt2 = (t2e.tv_nsec - t2b.tv_nsec) / NSECS + (t2e.tv_sec - t2b.tv_sec);
    tt2 += lt2;
    if (!quiet) printf("Time = %fs\n", lt2);

    if (pretty_print) {
      printf("STATBLOCK 2\n");
      for (int i = 0; i < X; ++i) {
        printf("Stat %d -> %d\n", i, statblock2[i]);
      }
      printf("---------------------\n");
    }
  
    if (!quiet) printf("Running method 3...\n");
  
  
    clock_gettime(CLOCK_MONOTONIC_RAW, &t3b);
    method3(statblock3);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t3e);
    lt3 = (t3e.tv_nsec - t3b.tv_nsec) / NSECS + (t3e.tv_sec - t3b.tv_sec);
    tt3 += lt3;
    if (!quiet) printf("Time = %fs\n", lt3);

  
    if (pretty_print) {
      printf("STATBLOCK 3\n");
      for (int i = 0; i < X; ++i) {
        printf("Stat %d -> %d\n", i, statblock3[i]);
      }
      printf("---------------------\n");
    }
  }
  printf("================================================\n");
  printf("\t\t Total \t\t\t Avg\n");
  printf("Method 1 \t %5.5f %s \t %5.5f %s\n", tt1 * units, unit_name, tt1/N * units, unit_name);
  printf("Method 2 \t %5.5f %s \t %5.5f %s\n", tt2 * units, unit_name, tt2/N * units, unit_name);
  printf("Method 3 \t %5.5f %s \t %5.5f %s\n", tt3 * units, unit_name, tt3/N * units, unit_name);
}

/* Method 1: Give out points one by one, to the highest number of a global roll */
void method1(int *stats)
{
  int r;
  for (int i = 0; i < P; ++i) {
    ++stats[rand() % X];
  }
}

/* Method 2: Give out points in P/X chunks with local rolls, then give out the rest using Method 1 */
void method2(int *stats)
{
  int p = P;
  int chunk_size = P/X;
  int r;

  while (p && chunk_size > 1) {
    PRINTD("p = %d, chunk_size = %d\n", p, chunk_size);
    for (int i = 0; i < X; ++i) {
      r = rand();
      stats[i] += r % chunk_size;
      p -= r % chunk_size;
      PRINTD("Giving %d points to idx %d\n", r % chunk_size, i);
    }
    chunk_size = p/X;
  }

  /* Revert back to Method 1 */
  for (int i = 0; i < p; ++i) {
    ++stats[rand() % X];
    --p;
    PRINTD("Giving 1 point to idx %d\n", max_idx);
  }
}

/* Method 3: Give out a point with a global roll with each iteration after giving out P/X chunks with local rolls that same iteration */
void method3(int *stats)
{
  int p = P;
  int chunk_size = P/X;
  int r;

  while (p) {
      chunk_size = p/X;
      PRINTD("p = %d, chunk_size = %d\n", p, chunk_size);
      for (int j = 0; j < X; ++j) {
        r = rand();
        /* Method 2: Give out a p/X chunk */
        if (chunk_size > 1) {
          stats[j] += r % chunk_size;
          p -= r % chunk_size;
          PRINTD("Giving %d points to idx %d\n", r % chunk_size, j);
        }
        /* Method 1: Give out 1 point */
      }
      ++stats[rand() % X];
      --p;
      PRINTD("Giving 1 point to idx %d\n", max_idx);
  }
}
