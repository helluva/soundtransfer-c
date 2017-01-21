

void init(int* length, char** decodedBytes);

// return 0: not receiving data
// return n: receiving data of byte length n
// return -1: transfer complete

int receive_frame(double frequency);
