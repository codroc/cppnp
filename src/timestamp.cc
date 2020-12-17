#include "timestamp.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include "declare.h"
Timestamp::Timestamp() :
    _microSecondsSinceEpoch(0){}
Timestamp::Timestamp(double microSeconds)
    : _microSecondsSinceEpoch(microSeconds){}

Timestamp::~Timestamp(){}

bool Timestamp::valid() { return _microSecondsSinceEpoch > 0.0; }

int64_t Timestamp::microSecondsSinceEpoch(){ return _microSecondsSinceEpoch; }

string Timestamp::ToString() const {
    char buf[32] = {0};
    long int sec = _microSecondsSinceEpoch / kMicroSecondsPerSecond;
    long int microsec = _microSecondsSinceEpoch % kMicroSecondsPerSecond;
    snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "\n", sec, microsec);
    return string(buf);
}

Timestamp Timestamp::Now(){ return Timestamp(NowMicroSeconds()); }
Timestamp Timestamp::NowAfter(double seconds) {
    return Timestamp(NowMicroSeconds() + seconds * kMicroSecondsPerSecond);
}

double Timestamp::NowMicroSeconds(){
    struct timeval tv;
    if(0 != gettimeofday(&tv, NULL))
        ::perror("gettimeofday");
    return tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec;
} 

// 因为 Timer 最后是要在 set 中进行排序的，所以一定要定义操作符 < ，不然 set 会报错
bool operator<(Timestamp l, Timestamp r) {
    return l.microSecondsSinceEpoch() < r.microSecondsSinceEpoch();
}
bool operator==(Timestamp l, Timestamp r) {
    return l.microSecondsSinceEpoch() == r.microSecondsSinceEpoch();
}
