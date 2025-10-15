#include <stdint.h>

template <typename T, uint8_t N>
class MedianN {
   public:
    T filter(T val) {
        _buf[_i] = val;
        if ((_i < N - 1) && (_buf[_i] > _buf[_i + 1])) {
            for (uint8_t i = _i; i < N - 1; i++) {
                if (_buf[i] > _buf[i + 1]) {
                    T t = _buf[i];
                    _buf[i] = _buf[i + 1];
                    _buf[i + 1] = t;
                }
            }
        } else {
            if ((_i > 0) && (_buf[_i - 1] > _buf[_i])) {
                for (uint8_t i = _i; i > 0; i--) {
                    if (_buf[i] < _buf[i - 1]) {
                        T t = _buf[i];
                        _buf[i] = _buf[i - 1];
                        _buf[i - 1] = t;
                    }
                }
            }
        }
        if (++_i >= N) _i = 0;
        return _buf[N / 2];
    }

    void init(T val) {
        for (uint8_t i = 0; i < N; i++) _buf[i] = val;
    }

   private:
    T _buf[N];
    uint8_t _i = 0;
};