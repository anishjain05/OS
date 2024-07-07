#include <math.h>

int main() {
    unsigned long long count = 1;
    unsigned long long count_to = pow(2,32);
    while (count < count_to) {
        count++;
    }
}