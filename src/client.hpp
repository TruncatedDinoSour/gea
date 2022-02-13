enum ClientMode { CLIENT_ADD, CLIENT_NORMAL };

class Client {
  public:
    int x, y = 0;
    char *filename = (char *)"unnamed";
    ClientMode mode = CLIENT_NORMAL;
};
