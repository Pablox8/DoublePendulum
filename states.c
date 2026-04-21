#include "states.h"
#include <math.h>

#define GRAVITY 9.81
#define EPSILON 1e-8

Pendulum make_pendulum(double m, double L) {
    return (Pendulum) { m, L };
}

State state_add(State Y1, State Y2) {
    State result = {
        .theta1 = Y1.theta1 + Y2.theta1,
        .omega1 = Y1.omega1 + Y2.omega1,
        .theta2 = Y1.theta2 + Y2.theta2,
        .omega2 = Y1.omega2 + Y2.omega2
    };
    return result;
}

State state_scale(State Y, double v) {
    State result = {
        .theta1 = Y.theta1 * v,
        .omega1 = Y.omega1 * v,
        .theta2 = Y.theta2 * v,
        .omega2 = Y.omega2 * v
    };
    return result;
}

// Equations:  https://www.physics.usyd.edu.au/~wheat/dpend_html/
// Derivation: https://scienceworld.wolfram.com/physics/DoublePendulum.html
State derivatives(State Y, Pendulum p1, Pendulum p2) {
    double delta = Y.theta2 - Y.theta1;
    double omega1_sqrd = Y.omega1 * Y.omega1;
    double omega2_sqrd = Y.omega2 * Y.omega2;

    double omega1_num = p2.m * p1.L * omega1_sqrd * sin(delta) * cos(delta) + p2.m * GRAVITY * sin(Y.theta2) * cos(delta) 
                      + p2.m * p2.L * omega2_sqrd * sin(delta) - (p1.m + p2.m) * GRAVITY * sin(Y.theta1);
    double omega1_den = (p1.m + p2.m) * p1.L - p2.m * p1.L * cos(delta) * cos(delta);

    double omega2_num = -p2.m * p2.L * omega2_sqrd * sin(delta) * cos(delta) + (p1.m + p2.m) 
                      * (GRAVITY * sin(Y.theta1) * cos(delta) - p1.L * omega1_sqrd * sin(delta) - GRAVITY * sin(Y.theta2));
    double omega2_den = (p1.m + p2.m) * p2.L - p2.m * p2.L * cos(delta) * cos(delta);

    State dYdt = {
        .theta1 = Y.omega1,
        .omega1 = (fabs(omega1_den) > EPSILON) ? (omega1_num / omega1_den) : 0,
        .theta2 = Y.omega2,
        .omega2 = (fabs(omega2_den) > EPSILON) ? (omega2_num / omega2_den) : 0
    };
    return dYdt;
}

// Explanation: https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods
State rk4(State Y, double dt, Pendulum p1, Pendulum p2) {
    State k1 = derivatives(Y, p1, p2);
    State k2 = derivatives(state_add(Y, state_scale(k1, dt/2)), p1, p2);
    State k3 = derivatives(state_add(Y, state_scale(k2, dt/2)), p1, p2);
    State k4 = derivatives(state_add(Y, state_scale(k3, dt)), p1, p2);

    State ki_sum = state_add(k1, state_scale(k2, 2));
    ki_sum = state_add(ki_sum, state_scale(k3, 2));
    ki_sum = state_add(ki_sum, k4);

    State Y_next = state_add(Y, state_scale(ki_sum, dt/6));
    return Y_next;
}