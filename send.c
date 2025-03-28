
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

#define INTERFACE "wlx503eaa3d660d" // Monitor mode interface

// Minimal Radiotap + 802.11 header
unsigned char header[] = {
    // Radiotap Header (8 bytes)
    0x00,
    0x00, // Version, Pad
    0x08,
    0x00, // Length
    0x00,
    0x00,
    0x00,
    0x00, // Present flags (none)

    // 802.11 Header (Data frame, To DS)
    0x08,
    0x01, // Frame control
    0x00,
    0x00, // Duration
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff, // Addr1: Broadcast
    0xaa,
    0xbb,
    0xcc,
    0xdd,
    0xee,
    0xff, // Addr2: Source
    0xaa,
    0xbb,
    0xcc,
    0xdd,
    0xee,
    0xff, // Addr3: BSSID
    0x00,
    0x00, // Sequence control

    // LLC + SNAP Header (8 bytes)
    0xaa,
    0xaa,
    0x03,
    0x00,
    0x00,
    0x00,
    // 0x08,
    // 0x06, // EtherType: ARP
    0xFF,
    0x01,
};

unsigned char payload[1028];

int main() {
  int sock;
  struct ifreq ifr;
  struct sockaddr_ll sa;

  // Create raw socket
  sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock < 0) {
    perror("Socket");
    exit(1);
  }

  // Get interface index
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, INTERFACE, IFNAMSIZ - 1);
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
  memcpy(packet, header, sizeof(header));
  memcpy(packet + sizeof(header), header, sizeof(header));

  for (int x = 0; x < 10; x++) {
    // Send the packet
    ssize_t result = sendto(sock, packet, sizeof(packet), 0,
                            (struct sockaddr *)&sa, sizeof(sa));
    if (result == -1) {
      perror("sendto");
      close(sock);
      exit(1);
    }
  }
  // printf("Injected %ld bytes on interface %s\n", result, INTERFACE);
  close(sock);
  return 0;
}
