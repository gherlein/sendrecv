
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define BUFSIZE 65536
const size_t MEGABYTE = 1024 * 1024;

#define SEND_IP 1

struct arp_header {
  uint16_t htype;        // Hardware type (1 for Ethernet)
  uint16_t ptype;        // Protocol type (0x0800 for IPv4)
  uint8_t hlen;          // Hardware address length (6 for MAC)
  uint8_t plen;          // Protocol address length (4 for IPv4)
  uint16_t opcode;       // Operation (1 for request, 2 for reply)
  uint8_t sender_mac[6]; // Sender MAC address
  uint8_t sender_ip[4];  // Sender IP address
  uint8_t target_mac[6]; // Target MAC address
  uint8_t target_ip[4];  // Target IP address
} __attribute__((packed));

void hexdump(const unsigned char *data, int len) {
  for (int i = 0; i < len; i++) {
    printf("%02x ", data[i]);
    if ((i + 1) % 16 == 0)
      printf("\n");
  }
  if (len % 16 != 0)
    printf("\n");
  printf("--------\n");
}

int main(int argc, char *argv[]) {
  int sock;
  struct ifreq ifr;
  struct sockaddr_ll sll;
  unsigned char buffer[BUFSIZE];
  uint32_t counter = 0;
  float bytes_recv = 0;

  if (argc < 2) {
    printf("useage: %s <interface>\n", argv[0]);
    return -1;
  }
  char *interface_str = argv[1];
  printf("opening %s...\n", interface_str); // Create raw socket

  sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock < 0) {
    perror("Socket");
    exit(1);
  }

  // Get interface index
  memset(&ifr, 0, sizeof(ifr));
  // strncpy(ifr.ifr_name, interface_str, IFNAMSIZ - 1);
  memcpy(ifr.ifr_name, interface_str, IFNAMSIZ - 1);
  if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
    perror("ioctl(SIOCGIFINDEX)");
    close(sock);
    exit(1);
  }

  // Bind to the interface
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = ifr.ifr_ifindex;
  sll.sll_protocol = htons(ETH_P_ALL);

  if (bind(sock, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
    perror("bind");
    close(sock);
    exit(1);
  }

  printf("Listening on interface %s...\n", interface_str);

  while (1) {
    ssize_t numbytes = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
    if (numbytes < 0) {
      perror("recvfrom");
      break;
    }
#ifdef SEND_IP
    if (buffer[59] == 0x08 && buffer[60] == 0x06) {
      // struct arp_header *arp = (struct arp_header *)(buffer + 61);
      counter++;
      bytes_recv += numbytes;
      printf("counter: %u - KBytes: %f\r", counter, bytes_recv / 1024);
      // printf("Sender IP: %d.%d.%d.%d\n", arp->sender_ip[0],
      // arp->sender_ip[1],
      //       arp->sender_ip[2], arp->sender_ip[3]);
    }
#endif
#ifdef SEND_ARP
    if (buffer[59] == 0x08 && buffer[60] == 0x06) {
      // struct arp_header *arp = (struct arp_header *)(buffer + 61);
      counter++;
      bytes_recv += numbytes;
      printf("counter: %u - KBytes: %f\r", counter, bytes_recv / 1024);
      // printf("Sender IP: %d.%d.%d.%d\n", arp->sender_ip[0],
      // arp->sender_ip[1],
      //       arp->sender_ip[2], arp->sender_ip[3]);
    }
#endif
#define SEND_RAW 1
#ifdef SEND_RAW
    if (buffer[0] == 0xFF && buffer[1] == 0xFE && buffer[2] == 0xFF &&
        buffer[3] == 0xFE) {
      counter++;
      bytes_recv += numbytes;
      printf("counter: %u - KBytes: %f\r", counter, bytes_recv / 1024);
    }
#endif
  }

  close(sock);
  return 0;
}
