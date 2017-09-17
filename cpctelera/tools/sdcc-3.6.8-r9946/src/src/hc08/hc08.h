typedef enum
  {
    SUB_HC08,
    SUB_S08
  }
HC08_SUB_PORT;

typedef struct
  {
    HC08_SUB_PORT sub;
  }
HC08_OPTS;

extern HC08_OPTS hc08_opts;

#define IS_HC08 (hc08_opts.sub == SUB_HC08)
#define IS_S08 (hc08_opts.sub == SUB_S08)

