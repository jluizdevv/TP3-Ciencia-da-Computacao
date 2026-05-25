#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct Node {
    int route_id;
    char *path;
    int path_len;
    struct Node *left;
    struct Node *right;
} Node;

Node* create_node(const char* bits, int len, int route_id) {
    Node* n = (Node*)malloc(sizeof(Node));
    n->path = (char*)malloc(len + 1);
    if (len > 0) strncpy(n->path, bits, len);
    n->path[len] = '\0';
    n->path_len = len;
    n->route_id = route_id;
    n->left = NULL;
    n->right = NULL;
    return n;
}

void insert(Node **root, const char *bits, int len, int route_id) {
    if (*root == NULL) {
        *root = create_node(bits, len, route_id);
        return;
    }
    
    Node *curr = *root;
    int i = 0;
    
    while (i < curr->path_len && i < len && curr->path[i] == bits[i]) {
        i++;
    }
    
    if (i == curr->path_len) {
        if (i == len) {
            curr->route_id = route_id;
        } else {
            if (bits[i] == '0') {
                insert(&(curr->left), bits + i, len - i, route_id);
            } else {
                insert(&(curr->right), bits + i, len - i, route_id);
            }
        }
    } else {
        Node *split = create_node(curr->path + i, curr->path_len - i, curr->route_id);
        split->left = curr->left;
        split->right = curr->right;

        curr->path[i] = '\0';
        curr->path_len = i;
        curr->route_id = -1;
        curr->left = NULL;
        curr->right = NULL;

        if (i == len) {
            curr->route_id = route_id;
            if (split->path[0] == '0') curr->left = split;
            else curr->right = split;
        } else {
            Node *new_leaf = create_node(bits + i, len - i, route_id);
            if (split->path[0] == '0') curr->left = split;
            else curr->right = split;

            if (bits[i] == '0') curr->left = new_leaf;
            else curr->right = new_leaf;
        }
    }
}

void ip_to_bits(const char *ip_str, char *bits, int *is_ipv6) {
    unsigned char buf[16];
    if (inet_pton(AF_INET, ip_str, buf)) {
        *is_ipv6 = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 7; j >= 0; j--) {
                bits[i * 8 + (7 - j)] = (buf[i] & (1 << j)) ? '1' : '0';
            }
        }
        bits[32] = '\0';
    } else if (inet_pton(AF_INET6, ip_str, buf)) {
        *is_ipv6 = 1;
        for (int i = 0; i < 16; i++) {
            for (int j = 7; j >= 0; j--) {
                bits[i * 8 + (7 - j)] = (buf[i] & (1 << j)) ? '1' : '0';
            }
        }
        bits[128] = '\0';
    }
}

void parse_cidr(const char *cidr, char *bits, int *len, int *is_ipv6) {
    char ip_str[256];
    strcpy(ip_str, cidr);
    char *slash = strchr(ip_str, '/');
    if (slash) {
        *slash = '\0';
        *len = atoi(slash + 1);
    } else {
        *len = -1;
    }
    
    ip_to_bits(ip_str, bits, is_ipv6);
    
    if (*len == -1) {
        *len = (*is_ipv6) ? 128 : 32;
    }
}

void insert_route(Node **root_v4, Node **root_v6, const char *cidr, int route_id) {
    char bits[129];
    int len, is_ipv6;
    parse_cidr(cidr, bits, &len, &is_ipv6);
    if (is_ipv6) {
        insert(root_v6, bits, len, route_id);
    } else {
        insert(root_v4, bits, len, route_id);
    }
}

int lookup(Node *root, const char *bits) {
    Node *curr = root;
    int best_route = -1;
    int i = 0;
    int bits_len = strlen(bits);

    while (curr != NULL) {
        int match_len = 0;
        while (match_len < curr->path_len && i + match_len < bits_len && curr->path[match_len] == bits[i + match_len]) {
            match_len++;
        }

        if (match_len == curr->path_len) {
            if (curr->route_id != -1) {
                best_route = curr->route_id;
            }
            i += curr->path_len;
            if (i == bits_len) break;

            if (bits[i] == '0') curr = curr->left;
            else curr = curr->right;
        } else {
            break;
        }
    }
    return best_route;
}

int lookup_route(Node *root_v4, Node *root_v6, const char *ip) {
    char bits[129];
    int is_ipv6;
    ip_to_bits(ip, bits, &is_ipv6);
    if (is_ipv6) {
        return lookup(root_v6, bits);
    } else {
        return lookup(root_v4, bits);
    }
}

void free_tree(Node* n) {
    if (n == NULL) return;
    free_tree(n->left);
    free_tree(n->right);
    free(n->path);
    free(n);
}

int main() {
    Node *root_v4 = NULL;
    Node *root_v6 = NULL;

    insert_route(&root_v4, &root_v6, "192.168.0.0/16", 10);
    insert_route(&root_v4, &root_v6, "192.168.1.0/24", 20);
    insert_route(&root_v4, &root_v6, "192.168.1.128/25", 30);
    insert_route(&root_v4, &root_v6, "10.0.0.0/8", 40);
    insert_route(&root_v4, &root_v6, "0.0.0.0/0", 50);
    insert_route(&root_v4, &root_v6, "2001:db8::/32", 100);
    insert_route(&root_v4, &root_v6, "2001:db8:a::/48", 200);

    const char* ips[] = {
        "192.168.0.50",
        "192.168.1.20",
        "192.168.1.150",
        "10.255.0.1",
        "8.8.8.8",
        "2001:db8:cafe::1",
        "2001:db8:a:b::1"
    };

    printf("--- Testes do Roteador (LPM) ---\n");
    for(int i = 0; i < 7; i++) {
        int res = lookup_route(root_v4, root_v6, ips[i]);
        printf("%-20s -> %d\n", ips[i], res);
    }

    free_tree(root_v4);
    free_tree(root_v6);

    return 0;
}