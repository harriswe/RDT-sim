#include "rdt.h"
#include <cstring>
#include <cstdio> // Include this for printf

const int WINDOW_SIZE = 8;
const int TIMEOUT_DURATION = 20; // You might need to adjust this
const int MAX_BUFFER_SIZE = 100; // Define a maximum buffer size

// Sender variables
int baseA = 0;
int nextSeqNumA = 0;
pkt sender_buffer[MAX_BUFFER_SIZE];
int sender_buffer_end = 0;
msg message_queue[MAX_BUFFER_SIZE];
int message_queue_end = 0;

// Receiver variables
int expectedSeqNumB = 0;

int calculateChecksum(const pkt &packet) {
    int checksum = 0;
    checksum += packet.seqnum;
    checksum += packet.acknum;
    for (int i = 0; i < 20; ++i) {
        checksum += (int)(packet.payload[i]);
    }
    return checksum;
}

void A_output(struct msg message) {
    if (nextSeqNumA < baseA + WINDOW_SIZE) {
        pkt packet;
        strncpy(packet.payload, message.data, sizeof(packet.payload));
        packet.seqnum = nextSeqNumA;
        packet.checksum = calculateChecksum(packet);

        sender_buffer[sender_buffer_end] = packet;
        sender_buffer_end = (sender_buffer_end + 1) % MAX_BUFFER_SIZE;
        to_network_layer(0, packet);

        printf("A_output: Sent packet %d\n", nextSeqNumA);

        if (baseA == nextSeqNumA) {
            printf("A_output: Starting timer for packet %d\n", baseA);
            starttimer(0, TIMEOUT_DURATION);
        }

        nextSeqNumA++;
    } else {
        printf("A_output: Window full, buffering message.\n");

        message_queue[message_queue_end] = message;
        message_queue_end = (message_queue_end + 1) % MAX_BUFFER_SIZE;
    }
}

void B_output(struct msg message) {
    // ... your implementation here ...
}

void A_input(struct pkt packet) {
    int receivedChecksum = packet.checksum;
    packet.checksum = 0;
    int recalculatedChecksum = calculateChecksum(packet);

    if (receivedChecksum != recalculatedChecksum || packet.acknum < baseA || packet.acknum >= baseA + WINDOW_SIZE) {
        printf("A_input: Corrupted or Invalid ACK packet received. Discarding.\n");
        return;
    }

    printf("A_input: Received valid ACK %d\n", packet.acknum);
    baseA = packet.acknum + 1;

    if (baseA < nextSeqNumA) {
        printf("A_input: Starting timer for packet %d\n", baseA);
        starttimer(0, TIMEOUT_DURATION);
    } else {
        printf("A_input: Stopping timer.\n");
        stoptimer(0);
    }

    // If there are more messages in the queue, send the next one
    while (message_queue_end != 0 && nextSeqNumA < baseA + WINDOW_SIZE) {
        msg nextMessage = message_queue[0];

        // Shift the remaining messages in the queue
        for (int i = 0; i < message_queue_end - 1; ++i) {
            message_queue[i] = message_queue[i + 1];
        }
        message_queue_end--;

        A_output(nextMessage);
    }
}

void A_timerinterrupt() {
    printf("A_timerinterrupt: Timeout occurred. Retransmitting packets from %d to %d\n", baseA, nextSeqNumA - 1);

    // Calculate the index of the first packet to retransmit
    int packetIndex = baseA % MAX_BUFFER_SIZE;

    for (int i = baseA; i < nextSeqNumA; ++i) {
        to_network_layer(0, sender_buffer[packetIndex]);
        printf("A_timerinterrupt: Retransmitted packet %d\n", sender_buffer[packetIndex].seqnum);
        packetIndex = (packetIndex + 1) % MAX_BUFFER_SIZE;
    }

    starttimer(0, TIMEOUT_DURATION);
}

void A_init() {
    baseA = 0;
    nextSeqNumA = 0;
}

void B_input(struct pkt packet) {
    int receivedChecksum = packet.checksum;
    packet.checksum = 0;
    int calculatedChecksum = calculateChecksum(packet);

    if (receivedChecksum != calculatedChecksum) {
        printf("B_input: Corrupted packet received. Discarding.\n");
        return;
    }

    if (packet.seqnum < expectedSeqNumB) {
        printf("B_input: Duplicate packet received. Discarding.\n");
        pkt ackPacket;
        ackPacket.acknum = expectedSeqNumB - 1;
        ackPacket.checksum = calculateChecksum(ackPacket);
        to_network_layer(1, ackPacket);
        printf("B_input: Sent duplicate ACK %d\n", ackPacket.acknum);
        return;
    }

    if (packet.seqnum == expectedSeqNumB) {
        printf("B_input: Received packet %d\n", packet.seqnum);
        to_app_layer(1, packet.payload);

        pkt ackPacket;
        ackPacket.acknum = expectedSeqNumB;
        ackPacket.checksum = calculateChecksum(ackPacket);
        to_network_layer(1, ackPacket);
        printf("B_input: Sent ACK %d\n", ackPacket.acknum);

        expectedSeqNumB++;
    } else {
        printf("B_input: Out-of-order packet received. Discarding.\n");
    }
}

void B_timerinterrupt() {
    // Not used in this implementation
}

void B_init() {
    expectedSeqNumB = 0;
}