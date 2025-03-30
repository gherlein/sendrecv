
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define SEND_IP 1

// Minimal Radiotap + 802.11 header
// clang-format off
unsigned char header[] = {
    // clang-format off
    // Radiotap Header (8 bytes)
    0x00, 0x00, // Version, pad
    0x08, 0x00, // Length
    0x00,0x00,0x00, 0x00, // Present flags (none)

    // 802.11 Header (Data frame, To DS)
    0x08,0x01, // Frame control
    0x00,0x00, // Duration
    0xff,0xff,0xff,0xff,0xff,0xff,
    // Addr1: Broadcast
    0xaa,0xbb, 0xcc,0xdd,0xee,0xff,

    // Addr2: Source
    0xaa, 0xbb, 0xcc, 0xdd, 0xee,0xff,

    // Addr3: BSSID
    0x00,
    0x00, // Sequence control

    // LLC + SNAP Header (8 bytes)
    0xaa,0xaa,0x03,0x00,0x00,0x00,

// EtherType: ARP is 0x0806, IPv4 is 0x0800
#ifdef SEND_IP
0x08, 0x00,
#endif
//0x08, 0x06,
//0xFF,0x01,
};

uint8_t ipv4header[] = {
       0x45, 0x00, 0x00, 0x3c, 0x1c, 0x46, 0x40, 0x00,
       0x40, 0x06, 0xb1, 0xe6, 0xc0, 0xa8, 0x01, 0x02,
       0xc0, 0xa8, 0x01, 0x01
   };

   uint8_t udp_packet[] = {
       // IPv4 Header (20 bytes)
       0x45,       // Version (4) + IHL (5)
       0x00,       // Type of Service
       0x00, 0x1C, // Total Length = 28 bytes
       0x00, 0x01, // Identification
       0x00, 0x00, // Flags + Fragment Offset
       0x40,       // TTL = 64
       0x11,       // Protocol = 17 (UDP)
       0x00, 0x00, // Header Checksum (we'll set this to 0 for simplicity)
       0xFF, 0xFF, 0xFF, 0xFF, // Source IP = 255.255.255.255 (broadcast)
       0xFF, 0xFF, 0xFF, 0xFF, // Dest IP   = 255.255.255.255 (broadcast)

       // UDP Header (8 bytes)
       0x13, 0x89, // Source Port = 5001
       0x13, 0x8A, // Destination Port = 5002
       0x00, 0x08, // Length = 8 (header only, no data)
       0x00, 0x00  // Checksum (set to 0 if not calculated)
   }; // clang-format on

unsigned char payload[1400];

int main(int argc, char *argv[]) {
  int sock;
  struct ifreq ifr;
  struct sockaddr_ll sa;
  uint32_t counter = 0;

  if (argc < 2) {
    printf("useage: %s <interface>\n", argv[0]);
    return -1;
  }
  char *interface_str = argv[1];
  printf("opening %s...\n", interface_str);

  // Create raw socket
  sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock < 0) {
    perror("Socket");
    exit(1);
  }

  // Get interface index
  memset(&ifr, 0, sizeof(ifr));
  // strncpy(ifr.ifr_name, INTERFACE, IFNAMSIZ - 1); // generates warnings
  memcpy(ifr.ifr_name, interface_str, IFNAMSIZ - 1);
  if (ioctl(sock, SIOCGIFINDEX, &ifr) == -1) {
    perror("ioctl(SIOCGIFINDEX)");
    close(sock);
    exit(1);
  }

  // Setup link-layer address struct
  memset(&sa, 0, sizeof(sa));
  sa.sll_ifindex = ifr.ifr_ifindex;
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(ETH_P_ALL);

  unsigned char packet[1500];

#ifdef SEND_IP
  memset(packet, 0x00, sizeof(packet));
  memcpy(packet, header, sizeof(header));
  memcpy(packet + sizeof(header), udp_packet, sizeof(udp_packet));
  for (int x = 0; x < sizeof(payload); x++) {
    // payload[x] = (uint8_t)(x & 0xFF);
    payload[x] = (0xFF);
  }
  memcpy(packet + sizeof(header) + sizeof(udp_packet), payload,
         sizeof(payload));
#endif
  // #define SEND_ARP
#ifdef SEND_ARP
  memset(packet, 0x00, sizeof(packet));
  memcpy(packet, header, sizeof(header));
  for (int x = 0; x < sizeof(payload); x++) {
    payload[x] = (uint8_t)(x & 0xFF);
  }
  memcpy(packet + sizeof(header), payload, sizeof(payload));
#endif
// #define RAW 1
#ifdef SEND_RAW
  memset(packet, 0x00, sizeof(packet));
  packet[0] = 0xff;
  packet[1] = 0xfe;
  packet[2] = 0xff;
  packet[3] = 0xfe;
  packet[4] = 0xff;
#endif
  while (1) {
    // send 10 frames
    for (int x = 0; x < 1000; x++) {
      // Send the packet
      ssize_t result = sendto(sock, packet, sizeof(packet), 0,
                              (struct sockaddr *)&sa, sizeof(sa));
      if (result == -1) {
        perror("sendto");
        close(sock);
        exit(1);
      }
      counter++;
      printf("counter: %u\r", counter);
    }
    printf("                                        \r");
    printf("sleeping...\r");
    sleep(2);
  }
  // printf("Injected %ld bytes on interface %s\n", result, INTERFACE);
  close(sock);
  return 0;
}
