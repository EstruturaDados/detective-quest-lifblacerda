#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Detective Quest - C implementation
// Binary tree for mansion rooms (static build)
// BST for clues (strings) and Hash table mapping clue -> suspect

#define HASH_SIZE 31
#define SUSPECT_HASH_SIZE 31
#define MAX_STR 64

/* ---------- Room binary tree ---------- */
typedef struct Room {
    char name[MAX_STR];
    char clue[MAX_STR];      // empty string if no clue in this room
    char suspect[MAX_STR];   // suspect associated with the clue (if any)
    int collected;           // whether clue was already collected
    struct Room *left, *right;
} Room;

Room* criarSala(const char *name, const char *clue, const char *suspect) {
    Room *r = (Room*)malloc(sizeof(Room));
    strncpy(r->name, name, MAX_STR-1); r->name[MAX_STR-1] = '\0';
    if (clue) strncpy(r->clue, clue, MAX_STR-1); else r->clue[0]='\0';
    if (suspect) strncpy(r->suspect, suspect, MAX_STR-1); else r->suspect[0]='\0';
    r->collected = 0;
    r->left = r->right = NULL;
    return r;
}

/* ---------- BST for clues ---------- */
typedef struct BSTNode {
    char clue[MAX_STR];
    struct BSTNode *left, *right;
} BSTNode;

BSTNode* bstInsert(BSTNode *root, const char *clue) {
    if (!root) {
        BSTNode *n = (BSTNode*)malloc(sizeof(BSTNode));
        strncpy(n->clue, clue, MAX_STR-1); n->clue[MAX_STR-1] = '\0';
        n->left = n->right = NULL;
        return n;
    }
    int cmp = strcmp(clue, root->clue);
    if (cmp < 0) root->left = bstInsert(root->left, clue);
    else if (cmp > 0) root->right = bstInsert(root->right, clue);
    // duplicates ignored
    return root;
}

void bstInOrder(BSTNode *root) {
    if (!root) return;
    bstInOrder(root->left);
    printf("- %s\n", root->clue);
    bstInOrder(root->right);
}

int bstSearch(BSTNode *root, const char *clue) {
    if (!root) return 0;
    int cmp = strcmp(clue, root->clue);
    if (cmp == 0) return 1;
    else if (cmp < 0) return bstSearch(root->left, clue);
    else return bstSearch(root->right, clue);
}

/* ---------- Hash table for clue -> suspect (chaining) ---------- */
typedef struct HashEntry {
    char clue[MAX_STR];
    char suspect[MAX_STR];
    struct HashEntry *next;
} HashEntry;

HashEntry *hashTable[HASH_SIZE];

unsigned int hash(const char *s) {
    unsigned int sum = 0;
    for (size_t i=0; i<strlen(s); ++i) sum += (unsigned char)s[i];
    return sum % HASH_SIZE;
}

void inserirNaHash(const char *clue, const char *suspect) {
    unsigned int h = hash(clue);
    HashEntry *node = (HashEntry*)malloc(sizeof(HashEntry));
    strncpy(node->clue, clue, MAX_STR-1); node->clue[MAX_STR-1] = '\0';
    strncpy(node->suspect, suspect, MAX_STR-1); node->suspect[MAX_STR-1] = '\0';
    node->next = hashTable[h];
    hashTable[h] = node;
}

void mostrarAssociacoes() {
    printf("\nAssociações (pista -> suspeito):\n");
    int found = 0;
    for (int i=0;i<HASH_SIZE;i++) {
        HashEntry *p = hashTable[i];
        while (p) {
            printf("%s -> %s\n", p->clue, p->suspect);
            found = 1;
            p = p->next;
        }
    }
    if (!found) printf("(Nenhuma associação cadastrada)\n");
}

/* ---------- Simple hash for suspect counts ---------- */
typedef struct SuspectCount {
    char suspect[MAX_STR];
    int count;
    struct SuspectCount *next;
} SuspectCount;

SuspectCount *suspectTable[SUSPECT_HASH_SIZE];

unsigned int hashSuspect(const char *s) {
    unsigned int sum = 0;
    for (size_t i=0; i<strlen(s); ++i) sum += (unsigned char)s[i];
    return sum % SUSPECT_HASH_SIZE;
}

void incrementSuspectCount(const char *suspect) {
    unsigned int h = hashSuspect(suspect);
    SuspectCount *p = suspectTable[h];
    while (p) {
        if (strcmp(p->suspect, suspect) == 0) { p->count++; return; }
        p = p->next;
    }
    // not found -> create
    SuspectCount *n = (SuspectCount*)malloc(sizeof(SuspectCount));
    strncpy(n->suspect, suspect, MAX_STR-1); n->suspect[MAX_STR-1] = '\0';
    n->count = 1;
    n->next = suspectTable[h];
    suspectTable[h] = n;
}

const char* suspeitoMaisCitado() {
    const char *best = NULL;
    int bestCount = 0;
    for (int i=0;i<SUSPECT_HASH_SIZE;i++) {
        SuspectCount *p = suspectTable[i];
        while (p) {
            if (p->count > bestCount) { bestCount = p->count; best = p->suspect; }
            p = p->next;
        }
    }
    return best;
}

/* ---------- Mansion build (static) ---------- */
Room* montarMansao() {
    // create rooms
    Room *hall = criarSala("Hall de Entrada", "", "");
    Room *salaEstar = criarSala("Sala de Estar", "pegada de lama", "Joaquim");
    Room *biblioteca = criarSala("Biblioteca", "bilhete misterioso", "Helena");
    Room *cozinha = criarSala("Cozinha", "faca com sangue", "Carlos");
    Room *jardim = criarSala("Jardim", "chave enferrujada", "Maria");
    Room *adega = criarSala("Adega", "frasco vazio", "Carlos");
    Room *escritorio = criarSala("Escritório", "impressão digital", "Helena");
    Room *quarto = criarSala("Quarto do Desaparecido", "bilhete misterioso", "Joaquim");

    // assemble tree
    hall->left = salaEstar;
    hall->right = biblioteca;

    salaEstar->left = cozinha;
    salaEstar->right = jardim;

    biblioteca->left = adega;
    biblioteca->right = escritorio;

    cozinha->left = quarto; // leaf beyond
    cozinha->right = NULL;

    // other leaves are NULL
    return hall;
}

/* ---------- Game exploration ---------- */
void explorarSalas(Room *root, BSTNode **bstRoot) {
    Room *curr = root;
    char op[8];
    printf("\nVocê está no Hall de Entrada. Use 'e' para esquerda, 'd' para direita, 's' para sair da exploração.\n");
    while (curr) {
        printf("Sala atual: %s\n", curr->name);
        // if there's a clue in this room and not yet collected -> collect
        if (curr->clue[0] != '\0' && !curr->collected) {
            printf("Você encontrou uma pista: %s (associada a '%s')\n", curr->clue, curr->suspect);
            if (!bstSearch(*bstRoot, curr->clue)) {
                *bstRoot = bstInsert(*bstRoot, curr->clue);
                inserirNaHash(curr->clue, curr->suspect);
                incrementSuspectCount(curr->suspect);
            }
            curr->collected = 1;
        }
        printf("Escolha (e/d/s): ");
        if (!fgets(op, sizeof(op), stdin)) break;
        // trim newline
        if (op[strlen(op)-1]=='\n') op[strlen(op)-1] = '\0';
        if (strcmp(op, "s") == 0) {
            printf("Saindo da exploração...\n");
            break;
        } else if (strcmp(op, "e") == 0) {
            if (curr->left) curr = curr->left;
            else { printf("Não há sala à esquerda. Fim de caminho.\n"); break; }
        } else if (strcmp(op, "d") == 0) {
            if (curr->right) curr = curr->right;
            else { printf("Não há sala à direita. Fim de caminho.\n"); break; }
        } else {
            printf("Opção inválida. Use 'e', 'd' ou 's'.\n");
        }
    }
}

/* ---------- Utilities ---------- */
void limparEstado() {
    for (int i=0;i<HASH_SIZE;i++) hashTable[i] = NULL;
    for (int i=0;i<SUSPECT_HASH_SIZE;i++) suspectTable[i] = NULL;
}

void liberarBST(BSTNode *root) {
    if (!root) return;
    liberarBST(root->left); liberarBST(root->right);
    free(root);
}

void liberarHash() {
    for (int i=0;i<HASH_SIZE;i++) {
        HashEntry *p = hashTable[i];
        while (p) { HashEntry *t = p; p = p->next; free(t); }
        hashTable[i] = NULL;
    }
    for (int i=0;i<SUSPECT_HASH_SIZE;i++) {
        SuspectCount *p = suspectTable[i];
        while (p) { SuspectCount *t = p; p = p->next; free(t); }
        suspectTable[i] = NULL;
    }
}

/* ---------- Main ---------- */
int main() {
    limparEstado();
    Room *mansao = montarMansao();
    BSTNode *clues = NULL;

    printf("=== Detective Quest - Mansion Investigation ===\n");
    explorarSalas(mansao, &clues);

    int running = 1;
    char cmd[8];
    while (running) {
        printf("\nMenu:\n");
        printf("1 - Mostrar pistas coletadas (ordem alfabética)\n");
        printf("2 - Mostrar associações pista -> suspeito\n");
        printf("3 - Mostrar suspeito mais citado\n");
        printf("4 - Sair\n");
        printf("Escolha: ");
        if (!fgets(cmd, sizeof(cmd), stdin)) break;
        if (cmd[strlen(cmd)-1]=='\n') cmd[strlen(cmd)-1] = '\0';
        if (strcmp(cmd, "1") == 0) {
            printf("\nPistas coletadas (em ordem):\n");
            if (clues) bstInOrder(clues); else printf("(Nenhuma pista coletada)\n");
        } else if (strcmp(cmd, "2") == 0) {
            mostrarAssociacoes();
        } else if (strcmp(cmd, "3") == 0) {
            const char *s = suspeitoMaisCitado();
            if (s) printf("\nSuspeito mais citado: %s\n", s);
            else printf("\nNenhum suspeito encontrado.\n");
        } else if (strcmp(cmd, "4") == 0) {
            running = 0;
        } else {
            printf("Opção inválida.\n");
        }
    }

    // liberar memória (simples)
    liberarBST(clues);
    liberarHash();
    // freeing rooms omitted for brevity (program exit)
    printf("Tchau! Obrigado por jogar Detective Quest.\n");
    return 0;
}
