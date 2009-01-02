typedef struct OpaqueJack *Jack;

Jack jack_init();
int process(jack_nframes_t nframes, void* arg);
void jack_shutdown_callback_jackdrum(void* arg);
void jack_error_callback_jackdrum(const char *msg);
void jack_free(Jack);
