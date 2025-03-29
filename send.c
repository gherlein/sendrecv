
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

// EtherType: ARP is 0x0806
  0x08, 0x06,
//0xFF,0x01,
};
// clang-format on

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
  memset(packet, 0x00, sizeof(packet));
  memcpy(packet, header, sizeof(header));
  for (int x = 0; x < sizeof(payload); x++) {
    payload[x] = (uint8_t)(x & 0xFF);
  }
  memcpy(packet + sizeof(header), payload, sizeof(payload));

  while (1) {
    // send 10 frames
    for (int x = 0; x < 10000; x++) {
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
