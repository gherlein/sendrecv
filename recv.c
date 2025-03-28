
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define INTERFACE "wlx5ca6e6a31052"
#define BUFSIZE 65536

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

int main() {
  int sock;
  struct ifreq ifr;
  struct sockaddr_ll sll;
  unsigned char buffer[BUFSIZE];

  // Create raw socket
  sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock < 0) {
    perror("Socket");
    exit(1);
  }

  // Get interface index
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, INTERFACE, IFNAMSIZ - 1);
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

  printf("Listening on interface %s...\n", INTERFACE);

  while (1) {
    ssize_t numbytes = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
    if (numbytes < 0) {
      perror("recvfrom");
      break;
    }

    printf("Received %zd bytes:\n", numbytes);
    hexdump(buffer, numbytes);
  }

  close(sock);
  return 0;
}
