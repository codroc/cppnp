#ifndef CPPNP_TIMESTAMP_H_
#define CPPNP_TIMESTAMP_H_ 

#include <string>
using namespace std;
class Timestamp{
public:
    Timestamp();
    Timestamp(double);
    ~Timestamp();

    bool valid();
    int64_t microSecondsSinceEpoch();
    string ToString() const;

    static Timestamp Now();
    static Timestamp NowAfter(double);
    static double NowMicroSeconds();
    static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
    int64_t _microSecondsSinceEpoch;
};

bool operator<(Timestamp l, Timestamp r);
bool operator==(Timestamp l, Timestamp r);
#endif // CPPNP_TIMESTAMP_H_
