#ifndef TIMER_H
#define TIMER_H

#include <map>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

typedef long long time_value;

struct timer_event {
    time_value times;
    void *arg;
    int eventtype;
};

time_value get_now();

class timer_link {
    static const int64_t mintimer = 1000 * 1;

   public:
    timer_link() {}

    virtual ~timer_link() {}

    // add new timer
    int add_timer(void *, time_value);

    //
    void *get_timer(time_value tnow);

    // remove timer
    void remote_timer(void *);

    time_value get_mintimer() const;

    int get_arg_time_size() const { return timer_arg_time.size(); }
    int get_time_arg_size() const { return timer_time_arg.size(); }

    void show() const;

   private:
    std::multimap<void *, time_value> timer_arg_time;

    std::multimap<time_value, void *> timer_time_arg;
};

#endif
