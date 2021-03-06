
void initialize_decoder(int* num_of_tones, unsigned char** decoded_bytes);

// return 0: not receiving data
// return n: receiving data of byte length n
// return -1: transfer complete

int receive_frame(double frequency);

void process_chunk(double* chunk);
void process_tone(int frequency);
void append_bits(unsigned char bits);
char* process_colors(unsigned char bits);
int compare_freq(double frequency, double target_frequency);

void process(int frequency);
int close_frequency(double freq);
