#pragma once

#define PACKETSIZE 256
#define BUFSIZE 252 // 256-4
#define POOLSIZE 10

#define EMOTIONSIZE 4


#define ACCEPT 1
#define READ   2
#define WRITE  3
#define POST   4
#define DISCONNECT 5

#define BROADCAST 0

#define ACTION_PACKET 11
#define EVENT_PACKET 21

#define LOBBY          0020
#define LOBBY_READY    0021 // 17
#define LOBBY_NO_READY 0022


#define DATA_NOT     0100
#define DATA_USER    0101
#define DATA_MONSTER 0110
#define DATA_EMOTION 0111

#define ERROR_SETTING  1000
#define ERROR_TRANSMIT 1001

#define QUIT   1111