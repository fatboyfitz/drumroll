typedef struct OpaqueJack *Jack;

int jackmidi_init(int num_notes);
void jackmidi_update_state(int note);
