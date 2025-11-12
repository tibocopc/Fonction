/*
 * Programme de calcul de dérivées symboliques
 * Supporte: +, -, *, /, ^, sin, cos, exp, ln
 * Exemple: x^2*sin(x) → 2*x*sin(x)+x^2*cos(x)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Types de nœuds dans l'arbre d'expression */
typedef enum {
    NODE_NUMBER,    // Constante numérique
    NODE_VARIABLE,  // Variable (x, u, v, z, etc.)
    NODE_ADD,       // Addition
    NODE_SUB,       // Soustraction
    NODE_MUL,       // Multiplication
    NODE_DIV,       // Division
    NODE_POW,       // Puissance
    NODE_SIN,       // Sinus
    NODE_COS,       // Cosinus
    NODE_EXP,       // Exponentielle
    NODE_LN         // Logarithme naturel
} NodeType;

/* Structure d'un nœud de l'arbre d'expression */
typedef struct Node {
    NodeType type;
    double value;          // Pour NODE_NUMBER
    char variable;         // Pour NODE_VARIABLE
    struct Node *left;     // Fils gauche
    struct Node *right;    // Fils droit
} Node;

/* Types de tokens pour le lexeur */
typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_POW,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SIN,
    TOKEN_COS,
    TOKEN_EXP,
    TOKEN_LN,
    TOKEN_END,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    double value;
    char variable;
} Token;

/* Variables globales pour le parsing */
static const char *input_str;
static int pos;
static Token current_token;

/* Prototypes de fonctions */
Node *create_node(NodeType type);
Node *create_number(double value);
Node *create_variable(char var);
Node *create_binary(NodeType type, Node *left, Node *right);
Node *create_unary(NodeType type, Node *child);
void free_tree(Node *node);
void print_tree(Node *node);
Node *differentiate(Node *node, char var);
Node *simplify(Node *node);
Node *copy_tree(Node *node);
int is_zero(Node *node);
int is_one(Node *node);
int is_constant(Node *node, char var);

/* Fonctions du lexeur */
void next_char(void);
void skip_whitespace(void);
Token get_next_token(void);

/* Fonctions du parseur */
Node *parse_expression(void);
Node *parse_term(void);
Node *parse_factor(void);
Node *parse_power(void);
Node *parse_primary(void);

/* === LEXEUR === */

void next_char(void) {
    pos++;
}

void skip_whitespace(void) {
    while (input_str[pos] == ' ' || input_str[pos] == '\t') {
        next_char();
    }
}

Token get_next_token(void) {
    Token token;
    skip_whitespace();
    
    if (input_str[pos] == '\0') {
        token.type = TOKEN_END;
        return token;
    }
    
    /* Nombres */
    if (isdigit(input_str[pos]) || input_str[pos] == '.') {
        char *endptr;
        token.type = TOKEN_NUMBER;
        token.value = strtod(&input_str[pos], &endptr);
        pos = endptr - input_str;
        return token;
    }
    
    /* Fonctions et variables */
    if (isalpha(input_str[pos])) {
        char buffer[10];
        int i = 0;
        while (isalpha(input_str[pos]) && i < 9) {
            buffer[i++] = input_str[pos++];
        }
        buffer[i] = '\0';
        
        if (strcmp(buffer, "sin") == 0) {
            token.type = TOKEN_SIN;
        } else if (strcmp(buffer, "cos") == 0) {
            token.type = TOKEN_COS;
        } else if (strcmp(buffer, "exp") == 0) {
            token.type = TOKEN_EXP;
        } else if (strcmp(buffer, "ln") == 0) {
            token.type = TOKEN_LN;
        } else if (strlen(buffer) == 1) {
            token.type = TOKEN_VARIABLE;
            token.variable = buffer[0];
        } else {
            token.type = TOKEN_ERROR;
        }
        return token;
    }
    
    /* Opérateurs */
    switch (input_str[pos]) {
        case '+':
            token.type = TOKEN_PLUS;
            next_char();
            break;
        case '-':
            token.type = TOKEN_MINUS;
            next_char();
            break;
        case '*':
            token.type = TOKEN_MULT;
            next_char();
            break;
        case '/':
            token.type = TOKEN_DIV;
            next_char();
            break;
        case '^':
            token.type = TOKEN_POW;
            next_char();
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            next_char();
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            next_char();
            break;
        default:
            token.type = TOKEN_ERROR;
            next_char();
    }
    
    return token;
}

/* === PARSEUR === */

/* expression = term (('+' | '-') term)* */
Node *parse_expression(void) {
    Node *left = parse_term();
    
    while (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS) {
        TokenType op = current_token.type;
        current_token = get_next_token();
        Node *right = parse_term();
        
        if (op == TOKEN_PLUS) {
            left = create_binary(NODE_ADD, left, right);
        } else {
            left = create_binary(NODE_SUB, left, right);
        }
    }
    
    return left;
}

/* term = factor (('*' | '/') factor)* */
Node *parse_term(void) {
    Node *left = parse_factor();
    
    while (current_token.type == TOKEN_MULT || current_token.type == TOKEN_DIV) {
        TokenType op = current_token.type;
        current_token = get_next_token();
        Node *right = parse_factor();
        
        if (op == TOKEN_MULT) {
            left = create_binary(NODE_MUL, left, right);
        } else {
            left = create_binary(NODE_DIV, left, right);
        }
    }
    
    return left;
}

/* factor = power */
Node *parse_factor(void) {
    return parse_power();
}

/* power = primary ('^' power)? */
Node *parse_power(void) {
    Node *left = parse_primary();
    
    if (current_token.type == TOKEN_POW) {
        current_token = get_next_token();
        Node *right = parse_power();
        left = create_binary(NODE_POW, left, right);
    }
    
    return left;
}

/* primary = NUMBER | VARIABLE | function '(' expression ')' | '(' expression ')' */
Node *parse_primary(void) {
    Node *node = NULL;
    
    if (current_token.type == TOKEN_NUMBER) {
        node = create_number(current_token.value);
        current_token = get_next_token();
    } else if (current_token.type == TOKEN_VARIABLE) {
        node = create_variable(current_token.variable);
        current_token = get_next_token();
    } else if (current_token.type == TOKEN_SIN || current_token.type == TOKEN_COS ||
               current_token.type == TOKEN_EXP || current_token.type == TOKEN_LN) {
        TokenType func = current_token.type;
        current_token = get_next_token();
        
        if (current_token.type != TOKEN_LPAREN) {
            fprintf(stderr, "Erreur: '(' attendu après fonction\n");
            exit(1);
        }
        current_token = get_next_token();
        
        Node *arg = parse_expression();
        
        if (current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Erreur: ')' attendu\n");
            exit(1);
        }
        current_token = get_next_token();
        
        if (func == TOKEN_SIN) {
            node = create_unary(NODE_SIN, arg);
        } else if (func == TOKEN_COS) {
            node = create_unary(NODE_COS, arg);
        } else if (func == TOKEN_EXP) {
            node = create_unary(NODE_EXP, arg);
        } else if (func == TOKEN_LN) {
            node = create_unary(NODE_LN, arg);
        }
    } else if (current_token.type == TOKEN_LPAREN) {
        current_token = get_next_token();
        node = parse_expression();
        
        if (current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Erreur: ')' attendu\n");
            exit(1);
        }
        current_token = get_next_token();
    } else {
        fprintf(stderr, "Erreur de syntaxe\n");
        exit(1);
    }
    
    return node;
}

/* === GESTION DES NŒUDS === */

Node *create_node(NodeType type) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->type = type;
    node->value = 0;
    node->variable = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

Node *create_number(double value) {
    Node *node = create_node(NODE_NUMBER);
    node->value = value;
    return node;
}

Node *create_variable(char var) {
    Node *node = create_node(NODE_VARIABLE);
    node->variable = var;
    return node;
}

Node *create_binary(NodeType type, Node *left, Node *right) {
    Node *node = create_node(type);
    node->left = left;
    node->right = right;
    return node;
}

Node *create_unary(NodeType type, Node *child) {
    Node *node = create_node(type);
    node->left = child;
    return node;
}

void free_tree(Node *node) {
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

Node *copy_tree(Node *node) {
    if (node == NULL) return NULL;
    
    Node *copy = create_node(node->type);
    copy->value = node->value;
    copy->variable = node->variable;
    copy->left = copy_tree(node->left);
    copy->right = copy_tree(node->right);
    
    return copy;
}

/* === AFFICHAGE === */

void print_tree(Node *node) {
    if (node == NULL) return;
    
    switch (node->type) {
        case NODE_NUMBER:
            if (node->value == (int)node->value) {
                printf("%d", (int)node->value);
            } else {
                printf("%.2f", node->value);
            }
            break;
            
        case NODE_VARIABLE:
            printf("%c", node->variable);
            break;
            
        case NODE_ADD:
            print_tree(node->left);
            printf("+");
            print_tree(node->right);
            break;
            
        case NODE_SUB:
            print_tree(node->left);
            printf("-");
            if (node->right->type == NODE_ADD || node->right->type == NODE_SUB) {
                printf("(");
                print_tree(node->right);
                printf(")");
            } else {
                print_tree(node->right);
            }
            break;
            
        case NODE_MUL:
            if (node->left->type == NODE_ADD || node->left->type == NODE_SUB) {
                printf("(");
                print_tree(node->left);
                printf(")");
            } else {
                print_tree(node->left);
            }
            printf("*");
            if (node->right->type == NODE_ADD || node->right->type == NODE_SUB) {
                printf("(");
                print_tree(node->right);
                printf(")");
            } else {
                print_tree(node->right);
            }
            break;
            
        case NODE_DIV:
            if (node->left->type == NODE_ADD || node->left->type == NODE_SUB) {
                printf("(");
                print_tree(node->left);
                printf(")");
            } else {
                print_tree(node->left);
            }
            printf("/");
            if (node->right->type != NODE_NUMBER && node->right->type != NODE_VARIABLE) {
                printf("(");
                print_tree(node->right);
                printf(")");
            } else {
                print_tree(node->right);
            }
            break;
            
        case NODE_POW:
            if (node->left->type != NODE_NUMBER && node->left->type != NODE_VARIABLE) {
                printf("(");
                print_tree(node->left);
                printf(")");
            } else {
                print_tree(node->left);
            }
            printf("^");
            if (node->right->type != NODE_NUMBER && node->right->type != NODE_VARIABLE) {
                printf("(");
                print_tree(node->right);
                printf(")");
            } else {
                print_tree(node->right);
            }
            break;
            
        case NODE_SIN:
            printf("sin(");
            print_tree(node->left);
            printf(")");
            break;
            
        case NODE_COS:
            printf("cos(");
            print_tree(node->left);
            printf(")");
            break;
            
        case NODE_EXP:
            printf("exp(");
            print_tree(node->left);
            printf(")");
            break;
            
        case NODE_LN:
            printf("ln(");
            print_tree(node->left);
            printf(")");
            break;
    }
}

/* === UTILITAIRES === */

int is_zero(Node *node) {
    return node != NULL && node->type == NODE_NUMBER && node->value == 0;
}

int is_one(Node *node) {
    return node != NULL && node->type == NODE_NUMBER && node->value == 1;
}

int is_constant(Node *node, char var) {
    if (node == NULL) return 1;
    
    if (node->type == NODE_NUMBER) return 1;
    if (node->type == NODE_VARIABLE) return node->variable != var;
    
    return is_constant(node->left, var) && is_constant(node->right, var);
}

/* === DÉRIVATION === */

Node *differentiate(Node *node, char var) {
    if (node == NULL) return NULL;
    
    switch (node->type) {
        case NODE_NUMBER:
            /* d/dx(c) = 0 */
            return create_number(0);
            
        case NODE_VARIABLE:
            /* d/dx(x) = 1, d/dx(y) = 0 */
            if (node->variable == var) {
                return create_number(1);
            } else {
                return create_number(0);
            }
            
        case NODE_ADD:
            /* d/dx(f + g) = f' + g' */
            return create_binary(NODE_ADD,
                                differentiate(node->left, var),
                                differentiate(node->right, var));
            
        case NODE_SUB:
            /* d/dx(f - g) = f' - g' */
            return create_binary(NODE_SUB,
                                differentiate(node->left, var),
                                differentiate(node->right, var));
            
        case NODE_MUL:
            /* d/dx(f * g) = f' * g + f * g' */
            return create_binary(NODE_ADD,
                                create_binary(NODE_MUL,
                                             differentiate(node->left, var),
                                             copy_tree(node->right)),
                                create_binary(NODE_MUL,
                                             copy_tree(node->left),
                                             differentiate(node->right, var)));
            
        case NODE_DIV:
            /* d/dx(f / g) = (f' * g - f * g') / g^2 */
            return create_binary(NODE_DIV,
                                create_binary(NODE_SUB,
                                             create_binary(NODE_MUL,
                                                          differentiate(node->left, var),
                                                          copy_tree(node->right)),
                                             create_binary(NODE_MUL,
                                                          copy_tree(node->left),
                                                          differentiate(node->right, var))),
                                create_binary(NODE_POW,
                                             copy_tree(node->right),
                                             create_number(2)));
            
        case NODE_POW:
            /* d/dx(f^n) = n * f^(n-1) * f' (si n est constant) */
            if (is_constant(node->right, var)) {
                return create_binary(NODE_MUL,
                                    create_binary(NODE_MUL,
                                                 copy_tree(node->right),
                                                 create_binary(NODE_POW,
                                                              copy_tree(node->left),
                                                              create_binary(NODE_SUB,
                                                                           copy_tree(node->right),
                                                                           create_number(1)))),
                                    differentiate(node->left, var));
            } else {
                /* Cas général: d/dx(f^g) = f^g * (g' * ln(f) + g * f'/f) */
                return create_binary(NODE_MUL,
                                    copy_tree(node),
                                    create_binary(NODE_ADD,
                                                 create_binary(NODE_MUL,
                                                              differentiate(node->right, var),
                                                              create_unary(NODE_LN, copy_tree(node->left))),
                                                 create_binary(NODE_MUL,
                                                              copy_tree(node->right),
                                                              create_binary(NODE_DIV,
                                                                           differentiate(node->left, var),
                                                                           copy_tree(node->left)))));
            }
            
        case NODE_SIN:
            /* d/dx(sin(f)) = cos(f) * f' */
            return create_binary(NODE_MUL,
                                create_unary(NODE_COS, copy_tree(node->left)),
                                differentiate(node->left, var));
            
        case NODE_COS:
            /* d/dx(cos(f)) = -sin(f) * f' */
            return create_binary(NODE_MUL,
                                create_binary(NODE_MUL,
                                             create_number(-1),
                                             create_unary(NODE_SIN, copy_tree(node->left))),
                                differentiate(node->left, var));
            
        case NODE_EXP:
            /* d/dx(exp(f)) = exp(f) * f' */
            return create_binary(NODE_MUL,
                                create_unary(NODE_EXP, copy_tree(node->left)),
                                differentiate(node->left, var));
            
        case NODE_LN:
            /* d/dx(ln(f)) = f' / f */
            return create_binary(NODE_DIV,
                                differentiate(node->left, var),
                                copy_tree(node->left));
    }
    
    return NULL;
}

/* === SIMPLIFICATION === */

Node *simplify(Node *node) {
    if (node == NULL) return NULL;
    
    /* Simplifier récursivement les sous-arbres */
    node->left = simplify(node->left);
    node->right = simplify(node->right);
    
    switch (node->type) {
        case NODE_ADD:
            /* 0 + x = x */
            if (is_zero(node->left)) {
                Node *result = node->right;
                free(node->left);
                free(node);
                return result;
            }
            /* x + 0 = x */
            if (is_zero(node->right)) {
                Node *result = node->left;
                free(node->right);
                free(node);
                return result;
            }
            /* c1 + c2 = c3 */
            if (node->left->type == NODE_NUMBER && node->right->type == NODE_NUMBER) {
                node->value = node->left->value + node->right->value;
                free(node->left);
                free(node->right);
                node->left = NULL;
                node->right = NULL;
                node->type = NODE_NUMBER;
            }
            break;
            
        case NODE_SUB:
            /* x - 0 = x */
            if (is_zero(node->right)) {
                Node *result = node->left;
                free(node->right);
                free(node);
                return result;
            }
            /* 0 - x = -x */
            if (is_zero(node->left)) {
                Node *result = create_binary(NODE_MUL, create_number(-1), node->right);
                free(node->left);
                free(node);
                return simplify(result);
            }
            /* c1 - c2 = c3 */
            if (node->left->type == NODE_NUMBER && node->right->type == NODE_NUMBER) {
                node->value = node->left->value - node->right->value;
                free(node->left);
                free(node->right);
                node->left = NULL;
                node->right = NULL;
                node->type = NODE_NUMBER;
            }
            break;
            
        case NODE_MUL:
            /* 0 * x = 0 */
            if (is_zero(node->left) || is_zero(node->right)) {
                free_tree(node->left);
                free_tree(node->right);
                node->left = NULL;
                node->right = NULL;
                node->type = NODE_NUMBER;
                node->value = 0;
            }
            /* 1 * x = x */
            else if (is_one(node->left)) {
                Node *result = node->right;
                free(node->left);
                free(node);
                return result;
            }
            /* x * 1 = x */
            else if (is_one(node->right)) {
                Node *result = node->left;
                free(node->right);
                free(node);
                return result;
            }
            /* c1 * c2 = c3 */
            else if (node->left->type == NODE_NUMBER && node->right->type == NODE_NUMBER) {
                node->value = node->left->value * node->right->value;
                free(node->left);
                free(node->right);
                node->left = NULL;
                node->right = NULL;
                node->type = NODE_NUMBER;
            }
            break;
            
        case NODE_DIV:
            /* 0 / x = 0 */
            if (is_zero(node->left)) {
                free_tree(node->right);
                node->right = NULL;
                node->type = NODE_NUMBER;
                node->value = 0;
            }
            /* x / 1 = x */
            else if (is_one(node->right)) {
                Node *result = node->left;
                free(node->right);
                free(node);
                return result;
            }
            /* c1 / c2 = c3 */
            else if (node->left->type == NODE_NUMBER && node->right->type == NODE_NUMBER) {
                if (node->right->value != 0) {
                    node->value = node->left->value / node->right->value;
                    free(node->left);
                    free(node->right);
                    node->left = NULL;
                    node->right = NULL;
                    node->type = NODE_NUMBER;
                }
            }
            break;
            
        case NODE_POW:
            /* x ^ 0 = 1 */
            if (is_zero(node->right)) {
                free_tree(node->left);
                free_tree(node->right);
                node->left = NULL;
                node->right = NULL;
                node->type = NODE_NUMBER;
                node->value = 1;
            }
            /* x ^ 1 = x */
            else if (is_one(node->right)) {
                Node *result = node->left;
                free(node->right);
                free(node);
                return result;
            }
            /* 0 ^ x = 0 (si x != 0) */
            else if (is_zero(node->left) && !is_zero(node->right)) {
                free_tree(node->right);
                node->right = NULL;
                node->type = NODE_NUMBER;
                node->value = 0;
            }
            /* 1 ^ x = 1 */
            else if (is_one(node->left)) {
                free_tree(node->right);
                node->right = NULL;
                node->type = NODE_NUMBER;
                node->value = 1;
            }
            /* c1 ^ c2 = c3 */
            else if (node->left->type == NODE_NUMBER && node->right->type == NODE_NUMBER) {
                node->value = pow(node->left->value, node->right->value);
                free(node->left);
                free(node->right);
                node->left = NULL;
                node->right = NULL;
                node->type = NODE_NUMBER;
            }
            break;
            
        default:
            break;
    }
    
    return node;
}

/* === PROGRAMME PRINCIPAL === */

int main(void) {
    char input[256];
    
    printf("=== Calculateur de dérivées symboliques ===\n");
    printf("Opérateurs supportés: +, -, *, /, ^\n");
    printf("Fonctions supportées: sin, cos, exp, ln\n");
    printf("Exemple: x^2*sin(x)\n\n");
    
    printf("Entrez une fonction: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Erreur de lecture\n");
        return 1;
    }
    
    /* Supprimer le retour à la ligne */
    input[strcspn(input, "\n")] = 0;
    
    /* Initialiser le parseur */
    input_str = input;
    pos = 0;
    current_token = get_next_token();
    
    /* Parser l'expression */
    Node *tree = parse_expression();
    
    if (current_token.type != TOKEN_END) {
        fprintf(stderr, "Erreur: caractères inattendus à la fin\n");
        free_tree(tree);
        return 1;
    }
    
    /* Afficher l'expression originale */
    printf("\nExpression: ");
    print_tree(tree);
    printf("\n");
    
    /* Calculer la dérivée par rapport à 'x' (ou première variable trouvée) */
    char var = 'x';
    Node *derivative = differentiate(tree, var);
    
    /* Simplifier la dérivée */
    derivative = simplify(derivative);
    
    /* Afficher la dérivée */
    printf("Dérivée d/d%c: ", var);
    print_tree(derivative);
    printf("\n");
    
    /* Libérer la mémoire */
    free_tree(tree);
    free_tree(derivative);
    
    return 0;
}
