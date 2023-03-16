#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIM_ALPHABET 64
#define MAX_COMMAND_SIZE 17
#define MAX_INT_SIZE 20
#define DIM_HASHTAB 5381

#define NEW_GAME "+nuova_partita"
#define SHOW_FILTERED "+stampa_filtrate"
#define INSERT_START "+inserisci_inizio"
#define INSERT_STOP "+inserisci_fine"

/* -------- STRUTTURE DATI -------- */
// Nodo della tabella di hash
typedef struct ht_node_ {
    struct ht_node_ *prev, *next;    // Puntatori al nodo precedente e a quello successivo
    char data[];                     // Parola memorizzata
} ht_node_t;

// Tabella di hash
typedef struct {
    ht_node_t *hashtab[DIM_HASHTAB];
    unsigned int size;
} chained_hashtable_t;

// Nodo dell'array str1 (vincoli 1-4-5)
typedef struct {
    int minimo;
    int esatto;
} str1_node_t;

// Nodo di str2 (vincoli 2-3)
typedef struct {
    char matched;
    int not_allowed_symb[DIM_ALPHABET];
} str2_node_t;

// Nodo dell'albero rosso-nero
typedef struct rbt_node_ {
    char color;
    struct rbt_node_ *p, *left, *right;
    char key[];
} rbt_node_t;

// Albero rosso-nero
typedef struct {
    rbt_node_t *root;
    rbt_node_t *nil;
} rbt_t;
/* -------- FINE STRUTTURE DATI -------- */

/* -------- PROTOTIPI DELLE FUNZIONI -------- */
// Funzioni relative alle tabelle di hash e alla manipolazione della chiave
unsigned long hash_funct (char word[], int len);
void hashtab_initialize (chained_hashtable_t **ht);
void hashtab_insert (chained_hashtable_t **ht, ht_node_t *node, int len);
void hashtab_insert_pos (chained_hashtable_t **ht, ht_node_t *node, int pos);
ht_node_t* hashtab_search (chained_hashtable_t *ht, char elem[], int len);
ht_node_t* hashtab_delete (chained_hashtable_t **ht, ht_node_t *node, int len);

// Funzioni per inserimento e visualizzazione dell'albero rosso-nero
rbt_t* set_rbt (void);
rbt_node_t* new_node(char *key, int len, rbt_node_t *nil);
void rb_insert (rbt_t **tree, rbt_node_t* z);
void rb_insert_fixup (rbt_t **tree, rbt_node_t **z);
rbt_t* left_rotate (rbt_t *tree, rbt_node_t *x);
rbt_t* right_rotate (rbt_t *tree, rbt_node_t *x);

// Funzioni di programma
int max (int n1, int n2);
int char_to_index(char c);
void string_matcher (char *r, char *p, char *res);
int occurrencesInString (const char *str, char c);
int occurrencesUntilPos (const char *s, char c, int limit);
int countValidOccurrences (const char *p, char *res, char c);
void update_constraints (char *p, char *r, char* res, int len, str1_node_t *str1, str2_node_t *str2);
int check_constraints (char *s, int len, str1_node_t *str1, str2_node_t *str2);

// Funzioni di programma con manipolazione delle strutture dati del dizionario
int checkConstraintsOnVocab (chained_hashtable_t **valid, chained_hashtable_t **notValid, int len, str1_node_t* str1, str2_node_t* str2);
int setAllWordsValid (chained_hashtable_t **valid, chained_hashtable_t **notValid, int len);
void printFilteredWords (chained_hashtable_t *ht, int len);

int main() {
    ht_node_t* node = NULL;
    int word_len, in_game = 0, continue_to_play = 1, i, j, max_attempts, remain_in_cycle, input_size, table1_valid = 1;
    str1_node_t str1[DIM_ALPHABET];
    char tmpstr[MAX_INT_SIZE];    // Serve per la lettura degli interi
    chained_hashtable_t *table1 = (chained_hashtable_t *) malloc(sizeof(chained_hashtable_t));
    chained_hashtable_t *table2 = (chained_hashtable_t *) malloc(sizeof(chained_hashtable_t));
    if (!table1 || !table2) {
        return -1;
    }
    hashtab_initialize(&table1);
    hashtab_initialize(&table2);

    if (fgets(tmpstr, MAX_INT_SIZE, stdin) == NULL) {
        return -1;
    }
    word_len = atoi(tmpstr);            // Lettura della lunghezza delle parole.
    input_size = max(word_len, MAX_COMMAND_SIZE) + 1;
    char p[word_len+1], r[word_len+1], res[word_len+1], input[input_size + 3];
    str2_node_t str2[word_len];

    // Inserimento delle parole nel vocabolario
    if (fgets(input, input_size + 3, stdin) == NULL) {
        return -1;
    }
    input[strlen(input) - 1] = '\0';
    while (strcmp(input, NEW_GAME) != 0) {      // Continua finché non viene impartito il comando '+nuova_partita'
        strncpy(p, input, word_len);
        if (p[0] != '+') {          // Se compare un comando, ignoralo
            p[word_len] = '\0';
            node = (ht_node_t *) malloc(sizeof(ht_node_t) + sizeof(char) * (word_len + 1));
            if (node) {
                strncpy(node->data, p, word_len);
                node->data[word_len] = '\0';
                node->prev = node->next = NULL;
                hashtab_insert(&table1, node, word_len); // All'inizio, la tabella valida è la 1
            }
        }
        if (fgets(input, input_size + 3, stdin) == NULL) {
            return -1;
        }
        input[strlen(input) - 1] = '\0';
    }

    // A questo punto, giunto il comando '+nuova_partita', si inizia a giocare.
    // Tutte le parole valide sono nella prima tabella al primo tentativo, poi si alternano.
    while (continue_to_play == 1) {
        // Inizializzazione delle strutture dati ausiliarie str1 e str2
        for (i = 0; i < DIM_ALPHABET; i++) {
            str1[i].minimo = 0;
            str1[i].esatto = 0;
        }
        for (i = 0; i < word_len; i++) {
            str2[i].matched = '*';
            for (j = 0; j < DIM_ALPHABET; j++) {
                str2[i].not_allowed_symb[j] = 0;
            }
        }

        // Lettura della parola di riferimento
        if (fgets(input, input_size + 3, stdin) == NULL) {
            return -1;
        }
        input[strlen(input) - 1] = '\0';
        strncpy(r, input, word_len);
        r[word_len] = '\0';

        // Lettura del numero di tentativi massimi possibili
        if (fgets(tmpstr, MAX_INT_SIZE, stdin) == NULL) {
            return -1;
        }
        max_attempts = atoi(tmpstr);

        // Inizio del gioco
        // Copia di tutte le parole presenti nella tabella 1 all'interno della tabella 2
        in_game = 1;
        while (in_game == 1) {
            // Lettura del comando/parola
            if (fgets(input, input_size + 3, stdin) == NULL) {
                return -1;
            }
            input[strlen(input) - 1] = '\0';
            if (strcmp(input, SHOW_FILTERED) == 0) {
                // Stampa delle parole filtrate in ordine lessicografico
                if (table1_valid) {
                    printFilteredWords(table1, word_len);
                } else {
                    printFilteredWords(table2, word_len);
                }
            } else if (strcmp(input, INSERT_START) == 0) {
                // Inserimento di nuove parole nel dizionario
                if (fgets(input, input_size + 3, stdin) == NULL) {
                    return -1;
                }
                input[strlen(input) - 1] = '\0';
                while (strcmp(input, INSERT_STOP) != 0) {
                    strncpy(p, input, word_len);
                    p[word_len] = '\0';
                    if (p[0] != '+') {
                        node = (ht_node_t *) malloc(sizeof(ht_node_t) + sizeof(char) * (word_len + 1));
                        if (node) {
                            strncpy(node->data, p, word_len);
                            node->data[word_len] = '\0';
                            if (check_constraints(node->data, word_len, str1, str2) == 1) {
                                if (table1_valid) {
                                    hashtab_insert(&table1, node, word_len);
                                } else {
                                    hashtab_insert(&table2, node, word_len);
                                }
                            } else {
                                if (table1_valid) {
                                    hashtab_insert(&table2, node, word_len);
                                } else {
                                    hashtab_insert(&table1, node, word_len);
                                }
                            }
                        }
                    }
                    if (fgets(input, input_size + 3, stdin) == NULL) {
                        return -1;
                    }
                    input[strlen(input) - 1] = '\0';
                }
            } else {
                strncpy(p, input, word_len);
                p[word_len] = '\0';
                if (strcmp(p, r) == 0) {
                    fprintf(stdout, "ok\n");
                    in_game = 0;
                } else {
                    if (hashtab_search(table1, p, word_len) == NULL &&
                        hashtab_search(table2, p, word_len) == NULL) {
                        fprintf(stdout, "not_exists\n");
                    } else {
                        string_matcher(r, p, res);
                        res[word_len] = '\0';
                        update_constraints(p, r, res, word_len, str1, str2);
                        if (table1_valid) {
                            checkConstraintsOnVocab(&table1, &table2, word_len, str1, str2);
                            fprintf(stdout, "%s\n%d\n", res, table1->size);
                        } else {
                            checkConstraintsOnVocab(&table2, &table1, word_len, str1, str2);
                            fprintf(stdout, "%s\n%d\n", res, table2->size);
                        }
                        max_attempts--;
                        if (max_attempts == 0) {
                            fprintf(stdout, "ko\n");
                            in_game = 0;
                        }
                    }
                }
            }
        }
        // Negazione logica della variabile
        if (table1_valid == 0) {
            table1_valid = 1;
        } else {
            table1_valid = 0;
        }
        remain_in_cycle = 1;
        while (remain_in_cycle == 1) {
            if (fgets(input, input_size + 3, stdin) == NULL) {
                // Lettura del carattere EOF (end-of-file)
                remain_in_cycle = 0;
                continue_to_play = 0;
            } else {
                input[strlen(input) - 1] = '\0';
                if (strcmp(input, NEW_GAME) == 0) {
                    // Inizio di una nuova partita
                    if (table1_valid) {
                        setAllWordsValid(&table1, &table2, word_len);
                    } else {
                        setAllWordsValid(&table2, &table1, word_len);
                    }
                    remain_in_cycle = 0;
                } else if (strcmp(input, INSERT_START) == 0) {
                    // Inserimento di nuove parole nel dizionario
                    if (fgets(input, input_size + 3, stdin) == NULL) {
                        return -1;
                    }
                    input[strlen(input) - 1] = '\0';
                    while (strcmp(input, INSERT_STOP) != 0) {
                        strncpy(p, input, word_len);
                        p[word_len] = '\0';
                        if (p[0] != '+') {
                            node = (ht_node_t *) malloc(sizeof(ht_node_t) + sizeof(char) * (word_len + 1));
                            if (node) {
                                strncpy(node->data, p, word_len);
                                node->data[word_len] = '\0';
                                if (table1_valid) {
                                    hashtab_insert(&table1, node, word_len);
                                } else {
                                    hashtab_insert(&table2, node, word_len);
                                }
                            }
                        }
                        if (fgets(input, input_size + 3, stdin) == NULL) {
                            return -1;
                        }
                        input[strlen(input) - 1] = '\0';
                    }
                }
            }
        }
    }
    return 0;
}

/* -------- FUNZIONI RIFERITE ALLA TABELLA DI HASH -------- */
/// Funzione: int strToKey (char word[], int len)
/// Restituisce la chiave univoca ottenuta dalla stringa word[]
unsigned long hash_funct (char word[], int len) {
    long int key = 0, base = 1;
    int i;
    for (i = len - 1; i >= 0; i--) {
        key += (char_to_index(word[i]) * base);
        base += DIM_ALPHABET;
    }
    return (key % DIM_HASHTAB);
}

/// Funzione: void hashtab_initialize (chained_hashtable_t **ht)
/// Pone tutti i puntatori della tabella di hash a NULL.
void hashtab_initialize (chained_hashtable_t **ht) {
    int i;
    for (i = 0; i < DIM_HASHTAB; i++) {
        (*ht)->hashtab[i] = NULL;
    }
    (*ht)->size = 0;
}

/// Funzione: void hashtab_insert (chained_hashtable_t **ht, ht_node_t *node, int len)
/// Inserisce nella tabella di hash ht il nodo (già allocato) node, contenente una parola di lunghezza len.
void hashtab_insert (chained_hashtable_t **ht, ht_node_t *node, int len) {
    ht_node_t **list = &(*ht)->hashtab[hash_funct(node->data, len)];
    node->next = (*list);
    if ((*list) != NULL) {
        (*list)->prev = node;
    }
    *list = node;
    node->prev = NULL;
    (*ht)->size++;
}

/// Funzione: void hashtab_insert_pos (chained_hashtable_t **ht, ht_node_t *node, int pos)
/// Inserisce nella tabella di hash ht il nodo (già allocato) node alla posizione pos, contenente una parola di lunghezza len.
/// Utile per le procedure di spostamento in cui è nota la posizione e non è necessario ricalcolarla.
void hashtab_insert_pos (chained_hashtable_t **ht, ht_node_t *node, int pos) {
    ht_node_t **list = &(*ht)->hashtab[pos];
    node->next = (*list);
    if ((*list) != NULL) {
        (*list)->prev = node;
    }
    *list = node;
    node->prev = NULL;
    (*ht)->size++;
}

/// Funzione: ht_node_t* hashtab_search (chained_hashtable_t *ht, char elem[], int len)
/// Ricerca all'interno della tabella di hash ht la stringa elem, di lunghezza len.
/// Restituisce il puntatore al nodo in caso di successo, altrimenti restituisce NULL.
ht_node_t* hashtab_search (chained_hashtable_t *ht, char elem[], int len) {
    ht_node_t *list = ht->hashtab[hash_funct(elem, len)];
    while (list && strcmp(list->data, elem) != 0) {
        list = list->next;
    }
    return list;
}

/// Funzione: ht_node_t* hashtab_delete (chained_hashtable_t **ht, ht_node_t *node, int len)
/// Rimuove e restituisce il nodo node dalla tabella di hash ht.
ht_node_t* hashtab_delete (chained_hashtable_t **ht, ht_node_t *node, int len) {
    ht_node_t **list = &(*ht)->hashtab[hash_funct(node->data, len)];
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        (*list) = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    (*ht)->size--;
    return node;
}

/* -------- FUNZIONI DI MANIPOLAZIONE E INTERROGAZIONE DELL'ALBERO ROSSO-NERO -------- */
/// Funzione: rbt_t* set_rbt (void)
/// Crea e ritorna l'istanza di un albero rosso-nero.
rbt_t* set_rbt (void) {
    rbt_t *tree = (rbt_t *) malloc(sizeof(rbt_node_t));
    if (tree == NULL)
        return NULL;
    tree->nil = (rbt_node_t *) malloc(sizeof(rbt_node_t) + sizeof(char) * 1);
    if (tree->nil == NULL)
        return NULL;
    strncpy(tree->nil->key, "\0", 1);
    tree->nil->color = 'B';
    tree->nil->p = NULL;
    tree->nil->left = NULL;
    tree->nil->right = NULL;
    tree->root = tree->nil;
    return tree;
}

/// Funzione: rbt_node_t* new_node(char *key, int len, rbt_node_t *nil)
/// Crea e alloca spazio per un nuovo nodo nell'albero rosso-nero, contenente la parola key di lunghezza len.
rbt_node_t* new_node(char *key, int len, rbt_node_t *nil) {
    rbt_node_t *n = (rbt_node_t *) malloc(sizeof(rbt_node_t) + sizeof (char) * (len + 1));
    if (n == NULL)
        return NULL;
    n->key[len] = '\0';
    strncpy(n->key, key, len);
    n->p = nil;
    n->left = nil;
    n->right = nil;
    return n;
}

/// Funzione: void rb_insert (rbt_t **tree, rbt_node_t* z)
/// Inserisce il nodo z nell'albero rosso-nero tree.
void rb_insert (rbt_t **tree, rbt_node_t* z) {
    rbt_node_t *y = (*tree)->nil;
    rbt_node_t *x = (*tree)->root;
    while (x != (*tree)->nil) {
        y = x;
        if (strcmp(z->key, x->key) < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }
    z->p = y;
    if (y == (*tree)->nil) {
        (*tree)->root = z;
    } else if (strcmp(z->key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }
    z->left = (*tree)->nil;
    z->right = (*tree)->nil;
    z->color = 'R';
    rb_insert_fixup(tree, &z);
}

/// Funzione: void rb_insert_fixup (rbt_t **tree, rbt_node_t **z)
/// Aggiusta l'albero tree a partire dal nodo z, in modo tale da garantire un bilanciamento parziale.
void rb_insert_fixup (rbt_t **tree, rbt_node_t **z) {
    rbt_node_t *y;
    while ((*z)->p->color == 'R') {
        if ((*z)->p == (*z)->p->p->left) {
            y = (*z)->p->p->right;
            if (y->color == 'R') {
                (*z)->p->color = 'B';
                y->color = 'B';
                (*z)->p->p->color = 'R';
                *z = (*z)->p->p;
            } else {
                if ((*z) == (*z)->p->right) {
                    (*z) = (*z)->p;
                    *tree = left_rotate(*tree, *z);
                }
                (*z)->p->color = 'B';
                (*z)->p->p->color = 'R';
                *tree = right_rotate (*tree, (*z)->p->p);
            }
        } else {
            y = (*z)->p->p->left;
            if (y->color == 'R') {
                (*z)->p->color = 'B';
                y->color = 'B';
                (*z)->p->p->color = 'R';
                *z = (*z)->p->p;
            } else {
                if ((*z) == (*z)->p->left) {
                    (*z) = (*z)->p;
                    *tree = right_rotate(*tree, *z);
                }
                (*z)->p->color = 'B';
                (*z)->p->p->color = 'R';
                *tree = left_rotate(*tree, (*z)->p->p);
            }
        }
    }
    (*tree)->root->color = 'B';
}

/// Funzione: rbt_t* left_rotate (rbt_t *tree, rbt_node_t *x)
/// Ruota a sinistra l'albero tree a partire dal nodo x; restituisce l'albero modificato.
rbt_t* left_rotate (rbt_t *tree, rbt_node_t *x) {
    rbt_node_t *y;
    if ((tree->root == NULL) || (x == tree->nil))
        return tree;
    if (x->right == tree->nil)
        return tree;
    y = x->right;
    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->p = x;
    }
    y->p = x->p;
    if (x->p == tree->nil) {
        tree->root = y;
    } else if (x == x->p->left) {
        x->p->left = y;
    } else {
        x->p->right = y;
    }
    y->left = x;
    x->p = y;
    return tree;
}

/// Funzione: rbt_t* right_rotate (rbt_t *tree, rbt_node_t *x)
/// Ruota a destra l'albero tree a partire dal nodo x; restituisce l'albero modificato.
rbt_t* right_rotate (rbt_t *tree, rbt_node_t *x) {
    rbt_node_t *y;
    if ((tree->root == NULL) || (x == tree->nil))
        return tree;
    if (x->left == tree->nil)
        return tree;
    y = x->left;
    x->left = y->right;
    if (y->right != tree->nil) {
        y->right->p = x;
    }
    y->p = x->p;
    if (x->p == tree->nil) {
        tree->root = y;
    } else if (x == x->p->right) {
        x->p->right = y;
    } else {
        x->p->left = y;
    }
    y->right = x;
    x->p = y;
    return tree;
}

/// Funzione: void inorder_print (rbt_node_t *root, rbt_node_t *nil)
/// Effettua una visita e stampa in ordine dell'albero rosso-nero.
void inorder_print (rbt_node_t *root, rbt_node_t *nil) {
    if (root != nil) {
        inorder_print(root->left, nil);
        fprintf(stdout, "%s\n", root->key);
        inorder_print(root->right, nil);
    }
}

/// Funzione: void deleteTree (rbt_node_t *root, rbt_node_t *nil)
/// Dealloca i nodi dell'albero rosso-nero.
void deleteTree (rbt_node_t *root, rbt_node_t *nil) {
    if (root != nil) {
        deleteTree(root->left, nil);
        deleteTree(root->right, nil);
        free(root);
    }
}

/* -------- FUNZIONI DI PROGRAMMA -------- */
/// Funzione: int max (int n1, int n2)
/// Restituisce il valore massimo tra n1 e n2.
int max (int n1, int n2) {
    if (n1 > n2) {
        return n1;
    }
    return n2;
}

/// Funzione: void string_matcher (char *r, char *p, char *res)
/// Confronta la parola di riferimento r con la parola inserita p e produce la stringa res.
void string_matcher (char *r, char *p, char *res) {
    int container[DIM_ALPHABET];
    int i, ind;
    for (i = 0; i < DIM_ALPHABET; i++) {
        container[i] = 0;
    }
    for (i = 0; r[i] != '\0'; i++) {
        ind = char_to_index(r[i]);
        container[ind]++;
    }
    for (i = 0; p[i] != '\0'; i++) {
        if (p[i] == r[i]) {
            res[i] = '+';
            ind = char_to_index(p[i]);
            container[ind]--;
        } else {
            res[i] = '?';
        }
    }
    for (i = 0; p[i] != '\0'; i++) {
        if (res[i] == '?') {
            ind = char_to_index(p[i]);
            if (container[ind] > 0) {
                res[i] = '|';
                container[ind]--;
            } else {
                res[i] = '/';
            }
        }
    }
}

/// Funzione: int char_to_index(char c)
/// Associa il carattere c ad un numero nell'intervallo [0...64].
int char_to_index(char c) {
    if (c == '-') {
        return 0;
    } else if (c >= '0' && c <= '9') {
        return ((int)c - (int)'0' + 1);
    } else if (c >= 'A' && c <= 'Z') {
        return ((int)c - (int)'A' + 11);
    } else if (c == '_') {
        return ((int)c - (int)'_' + 37);
    } else if (c >= 'a' && c <= 'z') {
        return ((int)c - (int)'a' + 38);
    } else {
        return 0;
    }
}

/// Funzione: int occurrencesInString (const char *s, char c)
/// Restituisce il numero di occorrenze del simbolo 'c' nella stringa s.
int occurrencesInString (const char *s, char c) {
    int n, i;
    for (i = 0, n = 0; s[i] != '\0'; i++) {
        if (s[i] == c) {
            n++;
        }
    }
    return n;
}

/// Funzione: int occurrencesUntilPos (const char *s, char c, int limit)
/// Conta le occorrenze di un carattere dall'inizio della posizione fino alla posizione limit.
int occurrencesUntilPos (const char *s, char c, int limit) {
    int n, i;
    for (i = 0, n = 0; s[i] != '\0' && i <= limit; i++) {
        if (s[i] == c) {
            n++;
        }
    }
    return n;
}

/// Funzione: int countValidOccurrences (const char *p, char *res, char c)
/// Conta le occorrenze valide del carattere c nella stringa p, ove res vale '+' oppure '|'.
int countValidOccurrences (const char *p, char *res, char c) {
    int i, n;
    for (i = 0, n = 0; p[i] != '\0'; i++) {
        if (p[i] == c && (res[i] == '+' || res[i] == '|')) {
            n++;
        }
    }
    return n;
}

/// Funzione: int check_constraints (char *s, int len, str1_node_t *str1, str2_node_t *str2)
/// Controlla se la stringa s soddisfa i vincoli presenti nelle strutture dati str1 e str2
/// Restituisce 1 se la parola è valida, altrimenti restituisce 0
int check_constraints (char *s, int len, str1_node_t *str1, str2_node_t *str2) {
    int i, container[DIM_ALPHABET];
    // Vincoli 1, 2, 3.
    for (i = 0; i < len; i++) {
        if ((str1[char_to_index(s[i])].minimo == -1) ||
            (str2[i].matched != '*' && str2[i].matched != s[i]) ||
            (str2[i].matched == '*' && str2[i].not_allowed_symb[char_to_index(s[i])] == 1)) {
            return 0;
        }
        container[char_to_index(s[i])]++;
    }
    // Vincoli 4, 5.
    for (i = 0; i < DIM_ALPHABET; i++) {
        container[i] = 0;
    }
    for (i = 0; i < len; i++) {
        container[char_to_index(s[i])]++;
    }
    for (i = 0; i < DIM_ALPHABET; i++) {
        if ((str1[i].esatto > 0 && str1[i].esatto != container[i]) ||
            (str1[i].esatto == 0 && str1[i].minimo > 0 && str1[i].minimo > container[i])) {
            return 0;
        }
    }
    return 1;
}

/// Funzione: void update_constraints (char *p, char *r, char* res, int len, str1_node_t *str1, str2_node_t *str2)
/// Aggiorna i vincoli contenuti nelle strutture str1 e str2 dopo il confronto tra p e r e la produzione della stringa
/// res.
void update_constraints (char *p, char *r, char* res, int len, str1_node_t *str1, str2_node_t *str2) {
    int i, matched_i, n;
    for (i = 0; i < len; i++) {
        matched_i = char_to_index(p[i]);
        if (res[i] == '+') {
            str2[i].matched = p[i];
            if (str1[matched_i].minimo > 0) {
                n = occurrencesUntilPos(p, p[i], i);
                if (str1[matched_i].esatto == 0) {
                    str1[matched_i].minimo = max(n, str1[matched_i].minimo);
                }
                else {
                    str1[matched_i].minimo = str1[matched_i].esatto;
                }
            } else {
                str1[matched_i].minimo = 1;
                str1[matched_i].esatto = 0;
            }
        } else if (res[i] == '|') {
            str2[i].not_allowed_symb[char_to_index(p[i])] = 1;
            if (str1[matched_i].minimo > 0) {
                n = occurrencesUntilPos(p, p[i], i);
                str1[matched_i].minimo = max(n, str1[matched_i].minimo);
            } else {
                str1[matched_i].minimo = 1;
                str1[matched_i].esatto = 0;
            }
        } else if (res[i] == '/') {
            if (occurrencesInString (r, p[i]) == 0) {
                str1[matched_i].minimo = -1;
                str1[matched_i].esatto = 0;
            } else {
                n = countValidOccurrences(p, res, p[i]);
                str1[matched_i].minimo = n;
                str1[matched_i].esatto = n;
                str2[i].not_allowed_symb[char_to_index(p[i])] = 1;
            }
        }
    }
}

/* -------- FUNZIONI DI PROGRAMMA CON MANIPOLAZIONE DEL DIZIONARIO -------- */
/// Funzione: int checkConstraintsOnVocab (chained_hashtable_t **valid, chained_hashtable_t **notValid, ...)
/// Sposta le parole che non soddisfano più i vincoli dalla tabella valid alla tabella notValid.
int checkConstraintsOnVocab (chained_hashtable_t **valid, chained_hashtable_t **notValid, int len, str1_node_t* str1, str2_node_t* str2) {
    int i, n = 0;
    ht_node_t *list = NULL, *temp = NULL;
    for (i = 0; i < DIM_HASHTAB; i++) {
        list = (*valid)->hashtab[i];
        while (list != NULL) {
            if (check_constraints(list->data, len, str1, str2) == 0) {
                temp = list;
                list = list->next;
                hashtab_delete(valid, temp, len);
                hashtab_insert_pos (notValid, temp, i);
                n++;
            } else {
                list = list->next;
            }
        }
    }
    return n;
}

/// Funzione: int setAllWordsValid (chained_hashtable_t **valid, chained_hashtable_t **notValid, int len)
/// Trasferisce tutte le parole contenute nella tabella notValid all'interno della tabella valid.
int setAllWordsValid (chained_hashtable_t **valid, chained_hashtable_t **notValid, int len) {
    int i, n = 0;
    ht_node_t *list = NULL, *temp = NULL;
    for (i = 0; i < DIM_HASHTAB; i++) {
        list = (*notValid)->hashtab[i];
        while (list != NULL) {
            temp = list;
            list = list->next;
            hashtab_delete(notValid, temp, len);
            hashtab_insert_pos (valid, temp, i);
            n++;
        }
    }
    return n;
}

/// Funzione: void printFilteredWords (chained_hashtable_t *ht, int len)
/// Stampa in ordine lessicografico le parole contenute nella tabella ht, creando un albero rosso-nero e facendo una
/// visita in ordine, deallocando poi il tutto.
void printFilteredWords (chained_hashtable_t *ht, int len) {
    int i;
    rbt_node_t *node = NULL;
    rbt_t *tree = NULL;
    ht_node_t *list = NULL;
    tree = set_rbt();
    for (i = 0; i < DIM_HASHTAB; i++) {
        for (list = ht->hashtab[i]; list != NULL; list = list->next) {
            node = new_node(list->data, len, tree->nil);
            if (node) {
                rb_insert(&tree, node);
            }
        }
    }
    inorder_print(tree->root, tree->nil);
    deleteTree(tree->root, tree->nil);
    free(tree);
}