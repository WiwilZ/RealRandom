# 真随机数实现
- linux
  通过读取 `"/dev/urandom"`（非阻塞方式）或`"/dev/random"`（阻塞方式）熵池获取随机字节
- windows
  通过`CryptGenRandom`API获取随机字节
无论是linux还是windows都涉及资源的开启和释放，在此采用RAII封装成单例类，对外提供`void random_bytes(void* buffer, size_t size)`以获取随机字节。

实现了
- `void random_bytes(void* buffer, size_t size)`
- `template <integral T> T random_integer()`
- `template <integral T, template <typename> class Container = std::pmr::vector> Container<T> random_integers(size_t size)`
- `template <typename Str = std::pmr::string> Str random_string(size_t size)`
接口。
随机整数模板参数只支持`bool`除外的整型类型，因为`bool`一般占8位，这8位中有1则为`true`，全0才为`false`，将这随机的1字节转为bool很大概率都是`true`。可以将随机整数的每一位看作随机`bool`。
