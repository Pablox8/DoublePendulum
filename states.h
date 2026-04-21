#ifndef STATES_H
#define STATES_H

typedef struct pendulum {
    double m;
    double L;
} Pendulum;

typedef struct state {
    double theta1;
    double omega1;
    double theta2;
    double omega2;
} State;

Pendulum make_pendulum(double, double);

State state_add(State, State);
State state_scale(State, double);

State derivatives(State, Pendulum, Pendulum);
State rk4(State, double, Pendulum, Pendulum);

#endif