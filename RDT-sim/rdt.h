//
// Created by Hans Dulimarta Fall 2024
//

#ifndef RDT_SIMULATION_RDT_H
#define RDT_SIMULATION_RDT_H

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
    char data[20];
};



/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

#ifdef __cplusplus
extern "C" {
#endif

void A_init();

void B_init();

void A_output(struct msg);

void B_output(struct msg);

void A_input(struct pkt);

void B_input(struct pkt);

void A_timerinterrupt();

void B_timerinterrupt();

void starttimer(int, float);

void stoptimer(int);

void to_network_layer(int, struct pkt);

void to_app_layer(int, char [20]);

#ifdef __cplusplus
}
#endif

#endif //RDT_SIMULATION_RDT_H
