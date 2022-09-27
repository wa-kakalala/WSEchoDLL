# WSEchoDLL

## API

- `int Bind(const char*ip, unsigned short* port)`
- `int Close()`
- `int Receive(void(*f)(const char*msg))`
- `void StopReceive()`
- `int Ping(const char*ip, unsigned short port)`
- `int Init()`
- `void Defer()`

