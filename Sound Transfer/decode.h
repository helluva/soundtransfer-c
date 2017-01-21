

void initialize();

// return 0: not receiving data
// return n: receiving data of byte length n
// return -1: transfer complete

int frame(double frequency, char** decodedBytes);
